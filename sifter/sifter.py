#!/usr/bin/python

import signal
import sys
import subprocess
import os
import threading
import time
import curses
import random
import argparse
import code
import copy
from ctypes import *
from struct import *
from collections import deque
from binascii import hexlify
from system import System
from logger import dump_artifacts
from disassembley import Disassembler
from gui import Gui
from tests import Tests

INJECTOR = "./injector"

OUTPUT = "./data/"
LOG  = OUTPUT + "log"
SYNC = OUTPUT + "sync"
TICK = OUTPUT + "tick"
LAST = OUTPUT + "last"

class ThreadState:
    pause = False
    run = True


def cstr2py(s):
    return ''.join([chr(x) for x in s])

def result_string(insn, result):
    s = "%30s %2d %2d %2d %2d (%s)\n" % (
            hexlify(insn), result.valid,
            result.length, result.signum,
            result.sicode, hexlify(cstr2py(result.raw_insn)))
    return s

class Injector:
    SYNTH_MODE_RANDOM = "r"
    SYNTH_MODE_BRUTE = "b"
    SYNTH_MODE_TUNNEL = "t"
    process = None
    settings = None
    command = None

    def __init__(self, command_args):
        if "-r" in command_args:
            self.synth_mode = self.SYNTH_MODE_RANDOM
        elif "-b" in command_args:
            self.synth_mode = self.SYNTH_MODE_BRUTE
        elif "-t" in command_args:
            self.synth_mode = self.SYNTH_MODE_TUNNEL
        self.args = args
        self.root = (os.geteuid() == 0)
        self.seed = random.getrandbits(32)

        self.settings = settings

    def start(self):
        self.command = "%s %s -%c -R %s -s %d" % \
                (
                    INJECTOR,
                    " ".join(self.settings.args),
                    self.settings.synth_mode,
                    "-0" if self.settings.root else "",
                    self.settings.seed
                )
        self.process = subprocess.Popen(
            "exec %s" % self.command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            preexec_fn=os.setsid
            )
        
    def stop(self):
        if self.process:
            try:
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
            except OSError:
                pass

    def increment_synth_mode(self):
        if self.synth_mode == self.SYNTH_MODE_BRUTE:
            self.synth_mode = self.SYNTH_MODE_RANDOM
        elif self.synth_mode == self.SYNTH_MODE_RANDOM:
            self.synth_mode = self.SYNTH_MODE_TUNNEL
        elif self.synth_mode == self.SYNTH_MODE_TUNNEL:
            self.synth_mode = self.SYNTH_MODE_BRUTE

class Poll:
    SIGILL = 4
    SIGSEGV = 11
    SIGFPE = 8
    SIGBUS = 7
    SIGTRAP = 5

    def __init__(self, ts, injector, tests, command_line, sync=False, low_mem=False, search_unk=True,
            search_len=False, search_dis=False, search_ill=False):
        self.ts = ts
        self.injector = injector
        self.T = tests
        self.poll_thread = None
        self.sync = sync
        self.low_mem = low_mem
        self.search_len = search_len
        self.search_unk = search_unk
        self.search_dis = search_dis
        self.search_ill = search_ill

        if self.sync:
            with open(SYNC, "w") as f:
                f.write("#\n")
                f.write("# %s\n" % command_line)
                f.write("# %s\n" % injector.command)
                f.write("#\n")
                f.write("# cpu:\n")
                cpu = System.get_cpu_info()
                for l in cpu:
                    f.write("# %s\n" % l)
                f.write("# %s  v  l  s  c\n" % (" " * 28))

    def start(self):
        self.poll_thread = threading.Thread(target=self.poll)
        self.poll_thread.start()

    def stop(self):
        self.poll_thread.join()
        while self.ts.run:
            time.sleep(.1)

    def poll(self):
        while self.ts.run:
            while self.ts.pause:
                time.sleep(.1)

            bytes_polled = self.injector.process.stdout.readinto(self.T.r)

            if bytes_polled == sizeof(self.T.r):
                self.T.ic = self.T.ic + 1

                error = False
                if self.T.r.valid:
                    if self.search_unk and not self.T.r.disas_known and self.T.r.signum != self.SIGILL:
                        error = True
                    if self.search_len and self.T.r.disas_known and self.T.r.disas_length != self.T.r.length:
                        error = True
                    if self.search_dis and self.T.r.disas_known \
                        and self.T.r.disas_length != self.T.r.length and self.T.r.signum != self.SIGILL:
                        error = True
                    if self.search_ill and self.T.r.disas_known and self.T.r.signum == self.SIGILL:
                        error = True
                if error:
                    insn = cstr2py(self.T.r.raw_insn)[:self.T.r.length]
                    r = copy.deepcopy(self.T.r)
                    self.T.al.appendleft(r)
                    if insn not in self.T.ad:
                        if not self.low_mem:
                            self.T.ad[insn] = r
                        self.T.ac = self.T.ac + 1
                        if self.sync:
                            with open(SYNC, "a") as f:
                                f.write(result_string(insn, self.T.r))
            else:
                if self.injector.process.poll() is not None:
                    self.ts.run = False
                    break


def cleanup(gui, poll, injector, ts, tests, command_line, args):
    ts.run = False
    if gui:
        gui.stop()
    if poll:
        poll.stop()
    if injector:
        injector.stop()

    curses.nocbreak();
    curses.echo()
    curses.endwin()

    dump_artifacts(tests, injector, command_line)

    if args.save:
        with open(LAST, "w") as f:
            f.write(hexlify(cstr2py(tests.r.raw_insn)))

    sys.exit(0)

def main():
    def exit_handler(signal, frame):
        cleanup(gui, poll, injector, ts, tests, command_line, args)

    injector = None
    poll = None
    gui = None

    command_line = " ".join(sys.argv)

    parser = argparse.ArgumentParser()
    parser.add_argument("--len", action="store_true", default=False,
            help="search for length differences in all instructions (instructions\
            that executed differently than the disassembler expected, or did not\
            exist when the disassembler expected them to)"
            )
    parser.add_argument("--dis", action="store_true", default=False,
            help="search for length differences in valid instructions (instructions\
            that executed differently than the disassembler expected)"
            )
    parser.add_argument("--unk", action="store_true", default=False,
            help="search for unknown instructions (instructions that the\
            disassembler doesn't know about but successfully execute)"
            )
    parser.add_argument("--ill", action="store_true", default=False,
            help="the inverse of --unk, search for invalid disassemblies\
            (instructions that do not successfully execute but that the\
            disassembler acknowledges)"
            )
    parser.add_argument("--tick", action="store_true", default=False,
            help="periodically write the current instruction to disk"
            )
    parser.add_argument("--save", action="store_true", default=False,
            help="save search progress on exit"
            )
    parser.add_argument("--resume", action="store_true", default=False,
            help="resume search from last saved state"
            )
    parser.add_argument("--sync", action="store_true", default=False,
            help="write search results to disk as they are found"
            )
    parser.add_argument("--low-mem", action="store_true", default=False,
            help="do not store results in memory"
            )
    parser.add_argument("injector_args", nargs=argparse.REMAINDER)

    args = parser.parse_args()

    injector_args = args.injector_args
    if "--" in injector_args: injector_args.remove("--")

    if not args.len and not args.unk and not args.dis and not args.ill:
        print "warning: no search type (--len, --unk, --dis, --ill) specified, results will not be recorded."
        raw_input()

    if args.resume:
        if "-i" in injector_args:
            print "--resume is incompatible with -i"
            sys.exit(1)

        if os.path.exists(LAST):
            with open(LAST, "r") as f:
                insn = f.read()
                injector_args.extend(['-i',insn])
        else:
            print "no resume file found"
            sys.exit(1)

    if not os.path.exists(OUTPUT):
        os.makedirs(OUTPUT)

    ts = ThreadState()
    signal.signal(signal.SIGINT, exit_handler)

    settings = Settings(args.injector_args)

    tests = Tests()

    injector = Injector(settings)
    injector.start()

    poll = Poll(ts, injector, tests, command_line, args.sync, 
                    args.low_mem, args.unk, args.len, args.dis, args.ill)
    poll.start()

    gui = Gui(ts, injector, tests, args.tick)
    gui.start()

    while ts.run:
        time.sleep(.1)

    cleanup(gui, poll, injector, ts, tests, command_line, args)

if __name__ == '__main__':
    main()

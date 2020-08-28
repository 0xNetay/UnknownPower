#!/usr/bin/python3.7

import signal
import sys
import os
import threading
import time
import curses
import argparse
import code
import copy
from ctypes import *
from struct import *
from collections import deque
from binascii import hexlify
from system import System
from logger import dump_artifacts
from disassembler import Disassembler
from gui import Gui
from tests import Tests
from injector import Injector
from utils import cstr2py, result_string
from files import SYNC, LAST
from poll import Poll


class ThreadState:
    pause = False
    run = True


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
            f.write(hexlify(cstr2py(tests.r.raw_insn)).decode())

    sys.exit(0)


def parse_args():
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

    if "--" in injector_args:
        injector_args.remove("--")

    return args, injector_args


def main():
    def exit_handler(signal, frame):
        cleanup(gui, poll, injector, ts, tests, command_line, args)

    injector = None
    poll = None
    gui = None

    command_line = " ".join(sys.argv)

    args, injector_args = parse_args()

    if not args.len and not args.unk and not args.dis and not args.ill:
        print("warning: no search type (--len, --unk, --dis, --ill) specified, results will not be recorded.")
        input()

    if args.resume:
        if "-i" in injector_args:
            print("--resume is incompatible with -i")
            sys.exit(1)

        if os.path.exists(LAST):
            with open(LAST, "r") as f:
                insn = f.read()
                injector_args.extend(['-i',insn])
        else:
            print("no resume file found")
            sys.exit(1)

    ts = ThreadState()
    signal.signal(signal.SIGINT, exit_handler)

    tests = Tests()

    injector = Injector(args.injector_args)
    injector.start()

    gui = Gui(ts, injector, tests, args.tick)

    poll = Poll(ts, injector, tests, gui, command_line, args.sync,
                args.low_mem, args.unk, args.len, args.dis, args.ill)

    poll.start()
    gui.start()

    while ts.run:
        time.sleep(.1)

    cleanup(gui, poll, injector, ts, tests, command_line, args)


if __name__ == '__main__':
    main()

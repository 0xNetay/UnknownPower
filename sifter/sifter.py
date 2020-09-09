#!/usr/bin/env python3.7

import signal
import sys
import os
import threading
import time
import argparse
import color
from logger import Logger
from tests import Tests
from injector import Injector
from utils import cstr2py, to_hex_string
from files import get_last_file_path
from poll import Poll
from errors import SifterException


class ThreadState:
    pause = False
    run = True


def cleanup(poll, injector, tests, command_line, args):
    if injector:
        injector.stop()

    if args.save:
        with open(get_last_file_path(), "w") as f:
            f.write(to_hex_string(tests.r.raw_insn))

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


def validate_args(args, injector_args):
    if not args.len and not args.unk and not args.dis and not args.ill:
        print("warning: no search type (--len, --unk, --dis, --ill) specified, results will not be recorded.")
        input()

    if args.resume:
        if "-i" in injector_args:
            print("--resume is incompatible with -i")
            sys.exit(1)

        if os.path.exists(get_last_file_path()):
            with open(get_last_file_path(), "r") as f:
                insn = f.read()
                injector_args.extend(['-i',insn])
        else:
            print("no resume file found")
            sys.exit(1)



def main():
    # Register to SIGINT
    def exit_handler(signal, frame):
        cleanup(poll, injector, tests, command_line, args)
    signal.signal(signal.SIGINT, exit_handler)

    # Get args
    command_line = " ".join(sys.argv)
    args, injector_args = parse_args()
    validate_args(args, injector_args)

    injector = Injector(args.injector_args)

    tests = Tests()
    logger = Logger(args.sync, tests)
    poll = Poll(injector, tests, logger, args.low_mem,
                args.unk, args.len, args.dis, args.ill)

    logger.print_state(command_line, injector)
    injector.start()
    poll.run()

    cleanup(poll, injector, tests, command_line, args)


if __name__ == '__main__':
    try:
        main()
    except SifterException as e:
        msg = color.red("Error!\n")
        msg += str(e)
        print(msg)

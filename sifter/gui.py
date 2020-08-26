
import threading
import time
import curses
import random
from ctypes import *
from struct import *
from collections import deque
from binascii import hexlify
from system import System
from disassembley import Disassembler


OUTPUT = "./data/"
TICK = OUTPUT + "tick"

# targeting python 2.6 support
def int_to_comma(x):
    if type(x) not in [type(0), type(0L)]:
        raise TypeError("Parameter must be an integer.")
    if x < 0:
        return '-' + int_to_comma(-x)
    result = ''
    while x >= 1000:
        x, r = divmod(x, 1000)
        result = ",%03d%s" % (r, result)
    return "%d%s" % (x, result)


class Gui:
    TIME_SLICE = .01
    GRAY_BASE = 50
    TICK_MASK = 0xff
    RATE_Q = 100
    RATE_FACTOR = 1000

    INDENT = 10

    GRAYS = 50

    BLACK = 1
    WHITE = 2
    BLUE =  3
    RED =   4
    GREEN = 5

    COLOR_BLACK = 16
    COLOR_WHITE = 17
    COLOR_BLUE =  18
    COLOR_RED =   19
    COLOR_GREEN = 20

    def __init__(self, ts, injector, tests, do_tick):
        self.ts = ts;
        self.injector = injector
        self.T = tests
        self.gui_thread = None
        self.do_tick = do_tick
        self.ticks = 0

        self.last_ins_count = 0
        self.delta_log = deque(maxlen=self.RATE_Q)
        self.time_log = deque(maxlen=self.RATE_Q)

        self.disas = Disassembler()

        self.stdscr = curses.initscr()
        curses.start_color()

        # doesn't work
        # self.orig_colors = [curses.color_content(x) for x in xrange(256)]

        curses.use_default_colors()
        curses.noecho()
        curses.cbreak()
        curses.curs_set(0)
        self.stdscr.nodelay(1)

        self.sx = 0
        self.sy = 0

        self.init_colors()

        self.stdscr.bkgd(curses.color_pair(self.WHITE))

        self.last_time = time.time()

    def init_colors(self):
        if curses.has_colors() and curses.can_change_color():
            curses.init_color(self.COLOR_BLACK, 0, 0, 0)
            curses.init_color(self.COLOR_WHITE, 1000, 1000, 1000)
            curses.init_color(self.COLOR_BLUE, 0, 0, 1000)
            curses.init_color(self.COLOR_RED, 1000, 0, 0)
            curses.init_color(self.COLOR_GREEN, 0, 1000, 0)

            # this will remove flicker, but gives boring colors
            '''
            self.COLOR_BLACK = curses.COLOR_BLACK
            self.COLOR_WHITE = curses.COLOR_WHITE
            self.COLOR_BLUE = curses.COLOR_BLUE
            self.COLOR_RED = curses.COLOR_RED
            self.COLOR_GREEN = curses.COLOR_GREEN
            '''

            for i in xrange(0, self.GRAYS):
                curses.init_color(
                        self.GRAY_BASE + i,
                        i * 1000 / (self.GRAYS - 1),
                        i * 1000 / (self.GRAYS - 1),
                        i * 1000 / (self.GRAYS - 1)
                        )
                curses.init_pair(
                        self.GRAY_BASE + i,
                        self.GRAY_BASE + i,
                        self.COLOR_BLACK
                        )

        else:
            self.COLOR_BLACK = curses.COLOR_BLACK
            self.COLOR_WHITE = curses.COLOR_WHITE
            self.COLOR_BLUE = curses.COLOR_BLUE
            self.COLOR_RED = curses.COLOR_RED
            self.COLOR_GREEN = curses.COLOR_GREEN

            for i in xrange(0, self.GRAYS):
                curses.init_pair(
                        self.GRAY_BASE + i,
                        self.COLOR_WHITE,
                        self.COLOR_BLACK
                        )

        curses.init_pair(self.BLACK, self.COLOR_BLACK, self.COLOR_BLACK)
        curses.init_pair(self.WHITE, self.COLOR_WHITE, self.COLOR_BLACK)
        curses.init_pair(self.BLUE, self.COLOR_BLUE, self.COLOR_BLACK)
        curses.init_pair(self.RED, self.COLOR_RED, self.COLOR_BLACK)
        curses.init_pair(self.GREEN, self.COLOR_GREEN, self.COLOR_BLACK)

    def gray(self, scale):
        if curses.can_change_color():
            return curses.color_pair(self.GRAY_BASE + int(round(scale * (self.GRAYS - 1))))
        else:
            return curses.color_pair(self.WHITE)

    def box(self, window, x, y, w, h, color):
        for i in xrange(1, w - 1):
            window.addch(y, x + i, curses.ACS_HLINE, color)
            window.addch(y + h - 1, x + i, curses.ACS_HLINE, color)
        for i in xrange(1, h - 1):
            window.addch(y + i, x, curses.ACS_VLINE, color)
            window.addch(y + i, x + w - 1, curses.ACS_VLINE, color)
        window.addch(y, x, curses.ACS_ULCORNER, color)
        window.addch(y, x + w - 1, curses.ACS_URCORNER, color)
        window.addch(y + h - 1, x, curses.ACS_LLCORNER, color)
        window.addch(y + h - 1, x + w - 1, curses.ACS_LRCORNER, color)

    def bracket(self, window, x, y, h, color):
        for i in xrange(1, h - 1):
            window.addch(y + i, x, curses.ACS_VLINE, color)
        window.addch(y, x, curses.ACS_ULCORNER, color)
        window.addch(y + h - 1, x, curses.ACS_LLCORNER, color)

    def vaddstr(self, window, x, y, s, color):
        for i in xrange(0, len(s)):
            window.addch(y + i, x, s[i], color)

    def draw(self):
        try:
            self.stdscr.erase()

            # constants
            left = self.sx + self.INDENT
            top = self.sy
            top_bracket_height = self.T.IL
            top_bracket_middle = self.T.IL / 2
            mne_width = 10
            op_width = 45
            raw_width = (16*2)

            # render log bracket
            self.bracket(self.stdscr, left - 1, top, top_bracket_height + 2, self.gray(1))

            # render logo
            self.vaddstr(self.stdscr, left - 3, top + top_bracket_middle - 5, "sand", self.gray(.2))
            self.vaddstr(self.stdscr, left - 3, top + top_bracket_middle + 5, "sifter", self.gray(.2))

            # refresh instruction log
            synth_insn = cstr2py(self.T.r.raw_insn)
            (mnemonic, op_str, size) = self.disas.disassemble_instruction(synth_insn)
            self.T.il.append(
                    (
                        mnemonic,
                        op_str,
                        self.T.r.length,
                        "%s" % hexlify(synth_insn)
                    )
                )

            # render instruction log
            try:
                for (i, r) in enumerate(self.T.il):
                    line = i + self.T.IL - len(self.T.il)
                    (mnemonic, op_str, length, raw) = r
                    if i == len(self.T.il) - 1:
                        # latest instruction
                        # mnemonic
                        self.stdscr.addstr(
                                top + 1 + line,
                                left,
                                "%*s " % (mne_width, mnemonic),
                                self.gray(1)
                                )
                        # operands
                        self.stdscr.addstr(
                                top + 1 + line,
                                left + (mne_width + 1),
                                "%-*s " % (op_width, op_str),
                                curses.color_pair(self.BLUE)
                                )
                        # bytes
                        if self.maxx > left + (mne_width + 1) + (op_width + 1) + (raw_width + 1):
                            self.stdscr.addstr(
                                    top + 1 + line,
                                    left + (mne_width + 1) + (op_width + 1),
                                    "%s" % raw[0:length * 2],
                                    self.gray(.9)
                                    )
                            self.stdscr.addstr(
                                    top + 1 +line,
                                    left + (mne_width + 1) + (op_width + 1) + length * 2,
                                    "%s" % raw[length * 2:raw_width],
                                    self.gray(.3)
                                    )
                    else:
                        # previous instructions
                        # mnemonic, operands
                        self.stdscr.addstr(
                                top + 1 + line,
                                left,
                                "%*s %-*s" % (mne_width, mnemonic, op_width, op_str), 
                                self.gray(.5)
                                )
                        # bytes
                        if self.maxx > left + (mne_width + 1) + (op_width + 1) + (raw_width + 1):
                            self.stdscr.addstr(
                                    top + 1 + line,
                                    left + (mne_width + 1) + (op_width + 1),
                                    "%s" % raw[0:length * 2],
                                    self.gray(.3)
                                    )
                            self.stdscr.addstr(
                                    top + 1 + line,
                                    left + (mne_width + 1) + (op_width + 1) + length * 2,
                                    "%s" % raw[length * 2:raw_width],
                                    self.gray(.1)
                                    )
            except RuntimeError:
                # probably the deque was modified by the poller
                pass

            # rate calculation
            self.delta_log.append(self.T.ic - self.last_ins_count)
            self.last_ins_count = self.T.ic
            ctime = time.time()
            self.time_log.append(ctime - self.last_time)
            self.last_time = ctime
            rate = int(sum(self.delta_log)/sum(self.time_log))

            # render timestamp
            if self.maxx > left + (mne_width + 1) + (op_width + 1) + (raw_width + 1):
                self.vaddstr(
                        self.stdscr,
                        left + (mne_width + 1) + (op_width + 1) + (raw_width + 1),
                        top + 1,
                        self.T.elapsed(),
                        self.gray(.5)
                        )

            # render injection settings
            self.stdscr.addstr(top + 1, left - 8, "%d" % self.injector.settings.root, self.gray(.1))
            self.stdscr.addstr(top + 1, left - 7, "%s" % System.get_bit_mode().value, self.gray(.1))
            self.stdscr.addstr(top + 1, left - 3, "%c" % self.injector.settings.synth_mode, self.gray(.5))

            # render injection results
            self.stdscr.addstr(top + top_bracket_middle, left - 6, "v:", self.gray(.5))
            self.stdscr.addstr(top + top_bracket_middle, left - 4, "%2x" % self.T.r.valid)
            self.stdscr.addstr(top + top_bracket_middle + 1, left - 6, "l:", self.gray(.5))
            self.stdscr.addstr(top + top_bracket_middle + 1, left - 4, "%2x" % self.T.r.length)
            self.stdscr.addstr(top + top_bracket_middle + 2, left - 6, "s:", self.gray(.5))
            self.stdscr.addstr(top + top_bracket_middle + 2, left - 4, "%2x" % self.T.r.signum)
            self.stdscr.addstr(top + top_bracket_middle + 3, left - 6, "c:", self.gray(.5))
            self.stdscr.addstr(top + top_bracket_middle + 3, left - 4, "%2x" % self.T.r.sicode)
            
            # render instruction count
            self.stdscr.addstr(top + top_bracket_height + 2, left, "#", self.gray(.5))
            self.stdscr.addstr(top + top_bracket_height + 2, left + 2, 
                    "%s" % (int_to_comma(self.T.ic)), self.gray(1))
            # render rate
            self.stdscr.addstr(top + top_bracket_height + 3, left, 
                    "  %d/s%s" % (rate, " " * min(rate / self.RATE_FACTOR, 100)), curses.A_REVERSE)
            # render artifact count
            self.stdscr.addstr(top + top_bracket_height + 4, left, "#", self.gray(.5))
            self.stdscr.addstr(top + top_bracket_height + 4, left + 2, 
                    "%s" % (int_to_comma(self.T.ac)), curses.color_pair(self.RED))

            # render artifact log
            if self.maxy >= top + top_bracket_height + 5 + self.T.UL + 2:

                # render artifact bracket
                self.bracket(self.stdscr, left - 1, top + top_bracket_height + 5, self.T.UL + 2, self.gray(1))

                # render artifacts
                try:
                    for (i, r) in enumerate(self.T.al):
                        y = top_bracket_height + 5 + i
                        insn_hex = hexlify(cstr2py(r.raw_insn))

                        # unexplainable hack to remove some of the unexplainable
                        # flicker on my console.  a bug in ncurses?  doesn't
                        # happen if using curses.COLOR_RED instead of a custom
                        # red.  doesn't happen if using a new random string each
                        # time; doesn't happen if using a constant string each
                        # time.  only happens with the specific implementation below.
						#TODO: on systems with limited color settings, this
						# makes the background look like random characters
                        random_string = ("%02x" % random.randint(0,100)) * (raw_width-2)
                        self.stdscr.addstr(top + 1 + y, left, random_string, curses.color_pair(self.BLACK))

                        self.stdscr.addstr(top + 1 + y, left + 1, 
                                "%s" % insn_hex[0:r.length * 2], curses.color_pair(self.RED))
                        self.stdscr.addstr(top + 1 + y, left + 1 + r.length * 2, 
                                "%s" % insn_hex[r.length * 2:raw_width], self.gray(.25))
                except RuntimeError:
                    # probably the deque was modified by the poller
                    pass

            self.stdscr.refresh()
        except curses.error:
            pass

    def start(self):
        self.gui_thread = threading.Thread(target=self.render)
        self.gui_thread.start()

    def stop(self):
        self.gui_thread.join()

    def checkkey(self):
        c = self.stdscr.getch()
        if c == ord('p'):
            self.ts.pause = not self.ts.pause
        elif c == ord('q'):
            self.ts.run = False
        elif c == ord('m'):
            self.ts.pause = True
            time.sleep(.1)
            self.injector.stop()
            self.injector.settings.increment_synth_mode()
            self.injector.start()
            self.ts.pause = False

    def render(self):
        while self.ts.run:
            while self.ts.pause:
                self.checkkey()
                time.sleep(.1)

            (self.maxy,self.maxx) = self.stdscr.getmaxyx()

            self.sx = 1
            self.sy = max((self.maxy + 1 - (self.T.IL + self.T.UL + 5 + 2))/2, 0)

            self.checkkey()

            synth_insn = cstr2py(self.T.r.raw_insn)

            if synth_insn and not self.ts.pause:
                self.draw()

            if self.do_tick:
                self.ticks = self.ticks + 1
                if self.ticks & self.TICK_MASK == 0:
                    with open(TICK, 'w') as f:
                        f.write("%s" % hexlify(synth_insn))

            time.sleep(self.TIME_SLICE)


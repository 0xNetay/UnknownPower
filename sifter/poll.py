
import time
import copy
from ctypes import sizeof
from system import System
from utils import cstr2py, result_string, to_hex_string
from errors import InjectorClosed


class Poll:
    SIGILL = 4
    SIGSEGV = 11
    SIGFPE = 8
    SIGBUS = 7
    SIGTRAP = 5

    def __init__(self, injector, tests, logger, low_mem=False, search_unk=True,
                 search_len=False, search_dis=False, search_ill=False):
        self.injector = injector
        self.T = tests
        self._logger = logger
        self.low_mem = low_mem
        self.search_len = search_len
        self.search_unk = search_unk
        self.search_dis = search_dis
        self.search_ill = search_ill

    def run(self):
        while True:
            bytes_polled = self.injector.process.stdout.readinto(self.T.r)

            if bytes_polled == sizeof(self.T.r):
                self.T.ic = self.T.ic + 1

                error = False
                if self.T.r.valid:
                    if (self.search_unk) and (not self.T.r.disas_known) and (self.T.r.signum != self.SIGILL):
                        error = True
                    if (self.search_len) and (self.T.r.disas_known) and (self.T.r.disas_length != self.T.r.length):
                        error = True
                    if self.search_dis and self.T.r.disas_known \
                        and (self.T.r.disas_length != self.T.r.length) and (self.T.r.signum != self.SIGILL):
                        error = True
                    if (self.search_ill) and (self.T.r.disas_known) and (self.T.r.signum == self.SIGILL):
                        error = True

                if error:
                    insn_hex = to_hex_string(self.T.r.raw_insn[:self.T.r.length])
                    r = copy.deepcopy(self.T.r)

                    if insn_hex not in self.T.ad:
                        if not self.low_mem:
                            self.T.ad[insn_hex] = r
                        self.T.ac = self.T.ac + 1

                        self._logger.on_unknown_instruction(self.T.r)
            else:
                if self.injector.process.poll() is not None:
                    raise InjectorClosed("Injector was suddenly closed")

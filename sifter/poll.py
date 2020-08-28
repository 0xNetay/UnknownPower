
import threading
import time
import copy
from ctypes import sizeof
from system import System
from utils import cstr2py, result_string, to_hex_string
from files import SYNC


class Poll:
    SIGILL = 4
    SIGSEGV = 11
    SIGFPE = 8
    SIGBUS = 7
    SIGTRAP = 5

    def __init__(self, ts, injector, tests, gui, command_line, sync=False, low_mem=False, search_unk=True,
                 search_len=False, search_dis=False, search_ill=False):
        self.ts = ts
        self.injector = injector
        self.T = tests
        self.gui = gui
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
                    insn_hex = to_hex_string(self.T.r.raw_insn[:self.T.r.length])
                    #import ipdb; ipdb.set_trace()
                    r = copy.deepcopy(self.T.r)
                    self.T.al.appendleft(r)
                    if insn_hex not in self.T.ad:
                        if not self.low_mem:
                            self.T.ad[insn_hex] = r
                        self.T.ac = self.T.ac + 1
                        if self.sync:
                            with open(SYNC, "a") as f:
                                f.write(result_string(insn_hex, self.T.r))
            else:
                if self.injector.process.poll() is not None:
                    #self.gui.stop()
                    print("Injector shut down")
                    self.ts.run = False
                    import ipdb; ipdb.set_trace()
                    break

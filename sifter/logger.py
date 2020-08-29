
import sys
import time
from files import SYNC
from system import System
from utils import result_string, to_hex_string
from files import LOG


class _Tee:
    def __init__(self, name, mode):
        self.file = open(name, mode)
        self.stdout = sys.stdout
        sys.stdout = self

    def __del__(self):
        sys.stdout = self.stdout
        self.file.close()

    def write(self, data):
        self.file.write(data)
        self.stdout.write(data)

    def flush(self):
        self.file.flush()
        self.stdout.flush()

class Logger:
    def __init__(self, sync, tests):
        self._tee = _Tee(LOG, "w")
        self._sync = sync
        self._tests = tests

    def __del__(self):
        self._tee.write(f"# insn tested:     {self._tests.ic}\n")
        self._tee.write(f"# unk insn found:  {self._tests.ac}\n")
        self._tee.write(f"# runtime:         {self._tests.elapsed()}\n")

    def print_state(self, command_line, injector):
        self._tee.write("#\n")
        self._tee.write(f"# {command_line}\n")
        self._tee.write(f"# {injector.command}\n")
        self._tee.write("#\n")
        self._tee.write(f"# seed:        {injector.seed}\n")
        self._tee.write(f"# arch:        {System.get_arch().value}\n")
        self._tee.write(f"# bit mode:    {System.get_bit_mode().value}\n")
        self._tee.write(f"# date:        {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
        self._tee.write("#\n")
        self._tee.write("# cpu:\n")

        cpu = System.get_cpu_info()

        for l in cpu:
            self._tee.write(f"# {l}\n")

        self._tee.write("# {}  v  l  s  c\n".format(" " * 28))

        if self._sync:
            with open(SYNC, "w") as f:
                f.write("#\n")
                f.write(f"# {command_line}\n")
                f.write(f"# {injector.command}\n")
                f.write("#\n")
                f.write("# cpu:\n")

                for l in cpu:
                    f.write(f"# {l}\n")
                f.write("# {}  v  l  s  c\n".format(" " * 28))

    def on_unknown_instruction(self, injector_results):
        insn_hex = to_hex_string(injector_results.raw_insn[:injector_results.length])
        result_str = result_string(insn_hex, injector_results)

        self._tee.write(result_str)

        if self._sync:
            with open(SYNC, "a") as f:
                f.write(result_str)

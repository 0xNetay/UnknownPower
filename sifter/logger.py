
import sys
import time
from system import System


OUTPUT = "./data/"
LOG  = OUTPUT + "log"


class Tee(object):
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


def dump_artifacts(r, injector, command_line):
    tee = Tee(LOG, "w")
    tee.write("#\n")
    tee.write("# %s\n" % command_line)
    tee.write("# %s\n" % injector.command)
    tee.write("#\n")
    tee.write("# insn tested: %d\n" % r.ic)
    tee.write("# artf found:  %d\n" % r.ac)
    tee.write("# runtime:     %s\n" % r.elapsed())
    tee.write("# seed:        %d\n" % injector.settings.seed)
    tee.write("# arch:        %s\n" % System.get_arch().value)
    tee.write("# bit mode:    %s\n" % System.get_bit_mode().value)
    tee.write("# date:        %s\n" % time.strftime("%Y-%m-%d %H:%M:%S"))
    tee.write("#\n")
    tee.write("# cpu:\n")

    cpu = System.get_cpu_info()
    for l in cpu:
        tee.write("# %s\n" % l) 

    tee.write("# %s  v  l  s  c\n" % (" " * 28))
    for k in sorted(list(r.ad)):
        v = r.ad[k]
        tee.write(result_string(k, v))


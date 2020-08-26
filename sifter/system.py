
import re
import subprocess
from enum import Enum


class Arch(Enum):
    IntelX86
    PowerPC
    ARM

class BitMode(Enum):
    BitMode32 = 32
    BitMode64 = 64


class System:
    @staticmethod
    def get_cpu_info():
        with open("/proc/cpuinfo", "r") as f:
            cpu = [l.strip() for l in f.readlines()[:7]]
        return cpu

    @staticmethod
    def get_arch() -> Arch:
        raise NotImplementedError()

    @staticmethod
    def get_bit_mode() -> BitMode:
        injector_pipe = subprocess.Popen(['file', INJECTOR],
                                         stdout=subprocess.PIPE,
                                         stderr=subprocess.PIPE)
        injector_bitness, errors = injector_pipe.communicate()

        bit_mode_str = re.search(r".*(..)-bit.*", injector_bitness).group(1)

        for b in BitMode:
            if bit_mode_str == b.value:
                return b


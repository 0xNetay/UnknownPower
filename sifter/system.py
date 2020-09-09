
import re
import subprocess
from enum import Enum
from functools import lru_cache


class Arch(Enum):
    IntelX86 = "x86"
    PowerPC = "ppc"
    ARM = "arm"

class BitMode(Enum):
    BitMode32 = "32"
    BitMode64 = "64"


class System:
    @staticmethod
    @lru_cache()
    def get_cpu_info():
        with open("/proc/cpuinfo", "r") as f:
            cpu = [l.strip() for l in f.readlines()[:7]]
        return cpu

    @staticmethod
    @lru_cache()
    def get_arch() -> Arch:
        injector_bitness = System._get_uname_process()

        arch_str = re.search(r"(.*)_(.*)\n", injector_bitness).group(1)

        for a in Arch:
            if arch_str == a.value:
                return a

    @staticmethod
    @lru_cache()
    def get_bit_mode() -> BitMode:
        injector_bitness = System._get_uname_process()

        bit_mode_str = re.search(r"(.*)_(.*)\n", injector_bitness).group(2)

        for b in BitMode:
            if bit_mode_str == b.value:
                return b

    @staticmethod
    def _get_uname_process():
        UNAME_FAILED_STR = "unknown\n"

        injector_pipe = subprocess.Popen(['uname', '-p'],
                                         stdout=subprocess.PIPE,
                                         stderr=subprocess.PIPE)
        uname_output, errors = injector_pipe.communicate()
        uname_output = uname_output.decode()
        errors = errors.decode()

        if (len(errors) > 0) or (uname_output == UNAME_FAILED_STR):
            raise Exception(f"Failed to run 'uname -p'\n"
                            f"Output: {uname_output}\n"
                            f"Error: {errors}")

        return uname_output

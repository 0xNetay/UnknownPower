
import random
import signal
import subprocess
import os
import ctypes
import pathlib
from system import System, Arch
from errors import InjectorDoesntExist
from files import get_injector_file_path


def _get_raw_instruction_size() -> int:
    insn_size_dict = \
    {
        Arch.IntelX86: 16,
        Arch.PowerPC: 4,
        Arch.ARM: 4
    }

    return insn_size_dict[System.get_arch()]

class InjectorResults(ctypes.Structure):
    _fields_ = [('disas_length', ctypes.c_int),
                ('disas_known', ctypes.c_int),
                ('raw_insn', ctypes.c_ubyte * _get_raw_instruction_size()),
                ('valid', ctypes.c_int),
                ('length', ctypes.c_int),
                ('signum', ctypes.c_int),
                ('sicode', ctypes.c_int),
                ('siaddr', ctypes.c_int),
		]


class Injector:
    _SYNTH_MODE_RANDOM = "r"
    _SYNTH_MODE_BRUTE = "b"
    _SYNTH_MODE_TUNNEL = "t"


    def __init__(self, command_args):
        self._validate_injector_bin()

        if "-r" in command_args:
            self.synth_mode = Injector._SYNTH_MODE_RANDOM
        elif "-b" in command_args:
            self.synth_mode = Injector._SYNTH_MODE_BRUTE
        elif "-t" in command_args:
            self.synth_mode = Injector._SYNTH_MODE_TUNNEL

        self.command_args = command_args
        self.root = (os.geteuid() == 0)
        self.seed = random.getrandbits(32)

        self.command = "%s %s -%c -R %s -s %d" % \
                (
                    get_injector_file_path(),
                    " ".join(self.command_args),
                    self.synth_mode,
                    "-0" if self.root else "",
                    0 #self.seed
                )

    def start(self):
        self.process = subprocess.Popen(
            "exec %s" % self.command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            stderr=subprocess.PIPE,
            preexec_fn=os.setsid
            )

    def stop(self):
        if self.process:
            try:
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
            except OSError:
                pass

    def increment_synth_mode(self):
        if self.synth_mode == Injector._SYNTH_MODE_BRUTE:
            self.synth_mode = Injector._SYNTH_MODE_RANDOM
        elif self.synth_mode == Injector._SYNTH_MODE_RANDOM:
            self.synth_mode = Injector._SYNTH_MODE_TUNNEL
        elif self.synth_mode == Injector._SYNTH_MODE_TUNNEL:
            self.synth_mode = Injector._SYNTH_MODE_BRUTE

    def _validate_injector_bin(self):
        if not os.path.isfile(get_injector_file_path()):
            raise InjectorDoesntExist.from_path(get_injector_file_path())

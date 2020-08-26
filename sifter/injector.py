
import signal
import subprocess
import os
import ctypes

INJECTOR = "./injector"


class InjectorResults(ctypes.Structure):
    _fields_ = [('disas_length', ctypes.c_int),
                ('disas_known', ctypes.c_int),
                ('raw_insn', ctypes.c_ubyte * 16),
                ('valid', ctypes.c_int),
                ('length', ctypes.c_int),
                ('signum', ctypes.c_int),
                ('sicode', ctypes.c_int),
                ('siaddr', ctypes.c_int),
		]


class Injector:
    process = None
    settings = None
    command = None

    def __init__(self, settings):
        self.settings = settings

    def start(self):
        self.command = "%s %s -%c -R %s -s %d" % \
                (
                    INJECTOR,
                    " ".join(self.settings.args),
                    self.settings.synth_mode,
                    "-0" if self.settings.root else "",
                    self.settings.seed
                )
        self.process = subprocess.Popen(
            "exec %s" % self.command,
            shell=True,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            preexec_fn=os.setsid
            )
        
    def stop(self):
        if self.process:
            try:
                os.killpg(os.getpgid(self.process.pid), signal.SIGTERM)
            except OSError:
                pass


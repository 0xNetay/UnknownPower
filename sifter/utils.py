
from binascii import hexlify


def cstr2py(s):
    return (''.join([chr(x) for x in s])).encode()

def result_string(insn, result):
    s = "%30s %2d %2d %2d %2d (%s)\n" % (
            hexlify(insn).decode(), result.valid,
            result.length, result.signum,
            result.sicode, hexlify(cstr2py(result.raw_insn)).decode())
    return s


from binascii import hexlify


def cstr2py(s):
    return (''.join([chr(x) for x in s])).encode()

def to_hex_string(s):
    return ''.join([hex(x)[2:] for x in s])

def result_string(insn_hex, result):
    s = "%30s %2d %2d %2d %2d (%s)\n" % (
            insn_hex, result.valid,
            result.length, result.signum,
            result.sicode, hexlify(result.raw_insn).decode())
    return s


from capstone import *
from system import System, BitMode


class Disassembler:
    def __init__(self):
        if System.get_bit_mode() == BitMode.BitMode64:
            self._md = Cs(CS_ARCH_X86, CS_MODE_64)
        else:
            self._md = Cs(CS_ARCH_X86, CS_MODE_32)

    def disassemble_instruction(self, b):
        try:
            (address, size, mnemonic, op_str) = next(self._md.disasm_lite(b, 0, 1))
        except StopIteration:
            mnemonic="(unk)"
            op_str=""
            size = 0
        return (mnemonic, op_str, size)


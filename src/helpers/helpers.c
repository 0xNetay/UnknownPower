#include "helpers.h"

bool is_prefix(uint8_t x)
{
    // TODO: Check if it is valid for POWERPC
	return 
		x==0xf0 || /* lock */
		x==0xf2 || /* repne / bound */
		x==0xf3 || /* rep */
		x==0x2e || /* cs / branch taken */
		x==0x36 || /* ss / branch not taken */
		x==0x3e || /* ds */
		x==0x26 || /* es */
		x==0x64 || /* fs */
		x==0x65 || /* gs */
		x==0x66 || /* data */
		x==0x67    /* addr */
#if __x86_64__
		|| (x>=0x40 && x<=0x4f) /* rex */
#endif
		;
}

int prefix_count(void)
{
	int i;
	for (i=0; i<MAX_INSN_LENGTH; i++) {
		if (!is_prefix(inj.i.bytes[i])) {
			return i;
		}
	}
	return i;
}

bool has_dup_prefix(void)
{
	int i;
	int byte_count[256];
	memset(byte_count, 0, 256*sizeof(int));

	for (i=0; i<MAX_INSN_LENGTH; i++) {
		if (is_prefix(inj.i.bytes[i])) {
			byte_count[inj.i.bytes[i]]++;
		}
		else {
			break;
		}
	}

	for (i=0; i<256; i++) {
		if (byte_count[i]>1) {
			return true;
		}
	}

	return false;
}

bool has_opcode(uint8_t* op)
{
	int i, j;
	for (i=0; i<MAX_INSN_LENGTH; i++) {
		if (!is_prefix(inj.i.bytes[i])) {
			j=0;
			do {
				if (i+j>=MAX_INSN_LENGTH || op[j]!=inj.i.bytes[i+j]) {
					return false;
				}
				j++;
			} while (op[j]);

			return true;
		}
	}
	return false;
}

bool has_prefix(uint8_t* pre)
{
	int i, j;
	for (i=0; i<MAX_INSN_LENGTH; i++) {
		if (is_prefix(inj.i.bytes[i])) {
			j=0;
			do {
				if (inj.i.bytes[i]==pre[j]) {
					return true;
				}
				j++;
			} while (pre[j]);
		}
		else {
			return false;
		}
	}
	return false;
}
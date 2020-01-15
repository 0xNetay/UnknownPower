#include "global.h"

void preamble(void)
{
#ifdef POWER_PC
    // TODO
#elif __x86_64__
	__asm__ __volatile__ ("\
			.global preamble_start                    \n\
			preamble_start:                           \n\
			pushfq                                    \n\
			orq %0, (%%rsp)                           \n\
			popfq                                     \n\
			.global preamble_end                      \n\
			preamble_end:                             \n\
			"
			:
			:"i"(TF)
			);
#else
	__asm__ __volatile__ ("\
			.global preamble_start                    \n\
			preamble_start:                           \n\
			pushfl                                    \n\
			orl %0, (%%esp)                           \n\
			popfl                                     \n\
			.global preamble_end                      \n\
			preamble_end:                             \n\
			"
			:
			:"i"(TF)
			);
#endif
}
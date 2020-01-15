#include "general.h"

#ifdef LINUX
typedef struct _disas __attribute__ ((packed)) {
#if RAW_REPORT_DISAS_MNE
	char mne[RAW_REPORT_DISAS_MNE_BYTES];
#endif
#if RAW_REPORT_DISAS_OPS
	char ops[RAW_REPORT_DISAS_OPS_BYTES];
#endif
#if RAW_REPORT_DISAS_LEN
	int len;
#endif
#if RAW_REPORT_DISAS_VAL
	int val;
#endif
} disassembly_info_t;
#endif
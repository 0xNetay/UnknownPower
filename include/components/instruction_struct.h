#pragma once

#include "config.h"

typedef struct _inst {
	uint8_t bytes[MAX_INSTRUCTION_LENGTH];
	int len; /* the number of specified bytes in the instruction */
} instruction_t;

typedef struct _inst_info {
	instruction_t i;
	int index;
	int last_len;
} instruction_info_t;
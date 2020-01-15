#pragma once

#include "general.h"

#ifdef LINUX
typedef struct __attribute__ ((packed)) _result {
	uint32_t valid;
	uint32_t length;
	uint32_t signum;
	uint32_t si_code;
	uint32_t addr;
} result_t;
#endif
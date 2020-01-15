#pragma once

#include "general.h"

#ifdef LINUX
struct {
	uint64_t dummy_stack_hi[256];
	uint64_t dummy_stack_lo[256];
} dummy_stack_t __attribute__ ((aligned(PAGE_SIZE)));
#endif
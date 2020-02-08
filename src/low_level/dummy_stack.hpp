#pragma once

#include "General.hpp"

#ifdef LINUX
struct  __attribute__ ((aligned(PAGE_SIZE))) DummyStack
{
	uint64_t dummy_stack_hi[256];
	uint64_t dummy_stack_lo[256];
};
#endif
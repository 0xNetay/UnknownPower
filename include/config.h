#pragma once

#include "general.h"

// Registers Size
#if __x86_64__
	#define IP REG_RIP 
#else
	#define IP REG_EIP 
#endif

// Flag to determine if to go over all bytes (true) or specific instructions (false)
#define USE_TF true

// Instruction Minimum Length. Intel is 1, PowerPC is 4
#define MIN_INSTRUCTION_LENGTH 4

// Instruction Maximum Length. Intel is 15, PowerPC is 4
#define MAX_INSTRUCTION_LENGTH 4


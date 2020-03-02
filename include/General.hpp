//
// Created by student on 1/29/20.
//

#ifndef UNKNOWNPOWER_GENERAL_HPP
#define UNKNOWNPOWER_GENERAL_HPP

#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

// Global Includes
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

// Linux-Only Includes
#include <unistd.h>
#include <execinfo.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <sys/wait.h>

#define STR(x) #x
#define XSTR(x) STR(x)

#define PAGE_SIZE 4096
#define TF        0x100

// Registers Size
#if __x86_64__
	#define IP REG_RIP 
#else
	#define IP REG_EIP 
#endif

// Flag to determine if to go over all bytes (true) or specific instructions (false)
#define USE_TF true

// Instruction Minimum Length. Intel is 1, PowerPC is 4
#if PROCESSOR == POWER_PC
#	define MIN_INSTRUCTION_LENGTH 4
#   define UNDEFINED_INSTRUCTION_LENGTH  4
#elif PROCESSOR == INTEL
#	define MIN_INSTRUCTION_LENGTH 1
#   define UNDEFINED_INSTRUCTION_LENGTH 2
#else
#   static_assert(false, "Unknown Processor");
#endif

// Instruction Maximum Length. Intel is 15, PowerPC is 4
#if PROCESSOR == POWER_PC
#	define MAX_INSTRUCTION_LENGTH 4
#	define JUMP_OR_UNDETERMINED_INSTRUCTION_LENGTH 4
#elif PROCESSOR == INTEL
#	define MAX_INSTRUCTION_LENGTH 15
#	define JUMP_OR_UNDETERMINED_INSTRUCTION_LENGTH 16
#endif

// Size of our blacklisted (ignored) opcodes array
#if PROCESSOR == POWER_PC
#	define MAX_BLACKLISTED_OPCODES  128
#	define MAX_BLACKLISTED_PREFIXES 16
#elif PROCESSOR == INTEL
#	define MAX_BLACKLISTED_OPCODES  128
#	define MAX_BLACKLISTED_PREFIXES 16
#endif

// Feedback
#define TICK_MASK 0xffff
#define RAW_REPORT_INSN_BYTES 16

#if USE_CAPSTONE
	#include <capstone/capstone.h>
	#if __x86_64__
		#define CS_MODE CS_MODE_64
	#else
		#define CS_MODE CS_MODE_32
	#endif

#	define RAW_REPORT_DISAS_MNE false
#	define RAW_REPORT_DISAS_MNE_BYTES 16
#	define RAW_REPORT_DISAS_OPS false
#	define RAW_REPORT_DISAS_OPS_BYTES 32
#	define RAW_REPORT_DISAS_LEN true
#	define RAW_REPORT_DISAS_VAL true
#endif

#endif //UNKNOWNPOWER_GENERAL_HPP
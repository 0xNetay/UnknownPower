#pragma once

#define _GNU_SOURCE

// Global Includes
#include <stdio.h>
#include <stdlib.h>
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

// #define LINUX

// Linux Include
#ifdef LINUX
#include <unistd.h>
#include <execinfo.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <sys/wait.h>
#endif

#define POWER_PC
// #define INTEL

#define STR(x) #x
#define XSTR(x) STR(x)

#define UD2_SIZE  2
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
#if defined(POWER_PC)
#	define MIN_INSTRUCTION_LENGTH 4
#elif defined(INTEL)
#	define MIN_INSTRUCTION_LENGTH 1
#endif

// Instruction Maximum Length. Intel is 15, PowerPC is 4
#if defined(POWER_PC)
#	define MAX_INSTRUCTION_LENGTH 4
#elif defined(INTEL)
#	define MAX_INSTRUCTION_LENGTH 15
#	define JUMP_INSTRUCTION_LENGTH 16
#endif

// Size of our blacklisted (ignored) opcodes array
#if defined(POWER_PC)
#	define MAX_BLACKLISTED_OPCODES  128
#	define MAX_BLACKLISTED_PREFIXES 16
#elif defined(INTEL)
#	define MAX_BLACKLISTED_OPCODES  128
#	define MAX_BLACKLISTED_PREFIXES 16
#endif

// Feedback
#define TICK_MASK 0xffff
#define RAW_REPORT_INSN_BYTES 16

#ifdef LINUX
#	define USE_CAPSTONE true
#else
#	define USE_CAPSTONE false
#endif

#if USE_CAPSTONE && defined(LINUX)
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
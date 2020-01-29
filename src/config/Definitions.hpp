//
// Created by student on 1/29/20.
//

#ifndef UNKNOWNPOWER_CONFIG_DEFINITIONS_HPP
#define UNKNOWNPOWER_CONFIG_DEFINITIONS_HPP

#include "general.hpp"

// ----------------
// Program Config
// ----------------

struct Config
{
    bool allow_exploring_more_than_one_prefix_in_search;
    int max_explored_prefix;
    int brute_force_byte_depth;
    long seed_for_random;
    int range_bytes;
    bool should_show_tick;
    int jobs_count;
    bool force_core;
    int core_count;
    bool enable_null_access;
    bool no_execute_support;
};

// ----------------
// Registers
// ----------------

// Struct of registers (x86 / x64)
// We set those registers to a specific state to return to.
struct RegState
{
#if defined(POWER_PC)
    uint32_t gpr0;
    uint32_t gpr1;
    uint32_t gpr2;
    uint32_t gpr3;
    uint32_t gpr4;
    uint32_t gpr5;
    uint32_t gpr6;
    uint32_t gpr7;
    uint32_t gpr8;
    uint32_t gpr9;
    uint32_t gpr10;
    uint32_t gpr11;
    uint32_t gpr12;
    uint32_t gpr13;
    uint32_t gpr14;
    uint32_t gpr15;
    uint32_t gpr16;
    uint32_t gpr17;
    uint32_t gpr18;
    uint32_t gpr19;
    uint32_t gpr20;
    uint32_t gpr21;
    uint32_t gpr22;
    uint32_t gpr23;
    uint32_t gpr24;
    uint32_t gpr25;
    uint32_t gpr26;
    uint32_t gpr27;
    uint32_t gpr28;
    uint32_t gpr29;
    uint32_t gpr30;
    uint32_t gpr31;

#elif defined(INTEL)
#	if __x86_64__
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rbp;
    uint64_t rsp;

#	else
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
#	endif
#endif
};

// ----------------
// Output
// ----------------

#ifdef LINUX
struct __attribute__ ((packed)) Result
{
    uint32_t valid;
    uint32_t length;
    uint32_t signum;
    uint32_t si_code;
    uint32_t address;
};
#endif

enum class OutputMode
{
    text,
    raw,
};

#ifdef LINUX
struct __attribute__ ((packed)) DisassemblyInfo
{
#if RAW_REPORT_DISAS_MNE
    char mne[RAW_REPORT_DISAS_MNE_BYTES];
#endif
#if RAW_REPORT_DISAS_OPS
    char ops[RAW_REPORT_DISAS_OPS_BYTES];
#endif
#if RAW_REPORT_DISAS_LEN
    int length;
#endif
#if RAW_REPORT_DISAS_VAL
    int value;
#endif
};
#endif

// ----------------
// Ignoring Opcodes
// ----------------

struct IgnoredOpcode
{
    char* opcode;
    char* reason;
};

struct IgnoredPrefix
{
    char* prefix;
    char* reason;
};

#endif //UNKNOWNPOWER_CONFIG_DEFINITIONS_HPP

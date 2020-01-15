#pragma once

#include "general.h"
#include "components/config_struct.h"
#include "components/registers_state_struct.h"
#include "components/build_mode.h"
#include "components/instruction_struct.h"
#include "components/output_mode.h"
#include "components/result_struct.h"
#include "components/ignore_opcode_struct.h"
#include "components/instruction_range_struct.h"
#include "helpers.h"

// Global Config
extern config_t             config;
extern registers_state_t    injected_reg_state;
extern ignore_opcode_t      opcode_blacklist[MAX_BLACKLISTED_OPCODES];
extern ignore_prefix_t      prefix_blacklist[MAX_BLACKLISTED_PREFIXES];

// Global Variables
extern instruction_info_t   current_instruction;
extern void*                packet_buffer;
extern char*                packet;
extern instruction_t       *range_marker;
extern instruction_range_t  search_range;
extern instruction_range_t  total_range;
extern int                  expected_length;

// Linux Only Variables
#ifdef LINUX
extern char         stack[SIGSTKSZ];
extern stack_t      ss;

extern mcontext_t   fault_context;

extern result_t     result;

extern pthread_mutex_t* pool_mutex;
extern pthread_mutex_t* output_mutex;
extern pthread_mutexattr_t mutex_attr;

#   if USE_CAPSTONE
extern csh      capstone_handle;
extern cs_insn *capstone_insn;
extern disassembly_info_t disas_info;
#   endif
#endif

// Constants
const search_mode_t     SEARCH_MODE = TUNNEL;
const output_mode_t     OUTPUT_MODE = TEXT;
const instruction_t     NULL_INSTRUCTION = {{0}, 0};

extern char debug, resume, preamble_start, preamble_end;
void preamble(void);

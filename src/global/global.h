#pragma once

#include "general.hpp"
#include "components/config_struct.hpp"
#include "components/registers_state_struct.hpp"
#include "components/build_mode.hpp"
#include "components/instruction_struct.hpp"
#include "components/output_mode.hpp"
#include "components/result_struct.hpp"
#include "components/ignore_opcode_struct.hpp"
#include "components/instruction_range_struct.hpp"
#include "components/disassembly_struct.hpp"
#include "helpers.h"

// Global Config
extern config_t             global_config;
extern registers_state_t    injected_reg_state;
extern ignore_opcode_t      opcode_blacklist[MAX_BLACKLISTED_OPCODES];
extern ignore_prefix_t      prefix_blacklist[MAX_BLACKLISTED_PREFIXES];

// Global Variables
extern instruction_info_t   current_instruction;
extern void*                packet_buffer;
extern char*                packet;
extern instruction_t       *current_range_length_marker;
extern instruction_range_t  current_search_range;
extern instruction_range_t  total_searched_range;
extern int                  current_expected_length;

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
extern disassembly_info_t disassembly_info;
#   endif
#endif

// Constants
const SearchMode        SEARCH_MODE = SearchMode::TunnelMinMax;
const output_mode_t     OUTPUT_MODE = TEXT;
const instruction_t     NULL_INSTRUCTION = {{0}, 0};

extern char debug, resume, preamble_start, preamble_end;
void preamble();

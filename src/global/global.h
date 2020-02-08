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


// For Output Manager
OutputMode _output_mode = OutputMode::raw;
inline void SetOutputMode(const OutputMode& output_mode) { _output_mode = output_mode; }
inline const OutputMode& GetOutputMode() { return _output_mode; }


// Global Variables
extern void*                packet_buffer;
extern char*                packet;

// Linux Only Variables
#ifdef LINUX
extern char         stack[SIGSTKSZ];
extern stack_t      ss;

extern mcontext_t   fault_context;

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

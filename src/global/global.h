#pragma once

#include "config.h"
#include "components/config_struct.h"
#include "components/registers_state_struct.h"
#include "components/build_mode.h"
#include "components/instruction_struct.h"

// Global Config
extern config_t             global_config;
extern registers_state_t    injected_reg_state;
extern search_mode_t        global_search_mode;
extern instruction_info_t   current_instruction;

// Global Variables
extern void* packet_buffer;
extern char* packet;

#ifdef LINUX
extern char stack[SIGSTKSZ];
extern stack_t ss;
#endif
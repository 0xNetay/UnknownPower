#include "global.h"

// CONFIG FOR APPLICATION
config_t global_config = {
    false,      // allow_exploring_more_than_one_prefix_in_search
	0,          // max_explored_prefix
	4,          // brute_force_byte_depth
	0,          // seed_for_random
	0,          // range_bytes
	false,      // should_show_tick
	1,          // jobs_count
	false,      // force_core
	0,          // core_count
	false,      // enable_null_access
	true,       // no_execute_support
};

// RANGES FOR PROCESSOR
#if defined(POWER_PC)
instruction_range_t total_range={
	.start={
		.bytes={0x00,0x00,0x00,0x00}, 
		.len=0
	},
	.end={
		.bytes={0xff,0xff,0xff,0xff}, 
		.len=0
	},
	.had_started=false
};
#elif defined(INTEL)
instruction_range_t total_range={
	.start={
		.bytes={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, 
		.len=0},
	.end={
		.bytes={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}, 
		.len=0},
	.had_started=false
};
#endif

// REGISTERS STATE FOR PROCESSOR
#if defined(POWER_PC)
registers_state_t injected_reg_state = {
	.gpr0 = 0,
	.gpr1 = 0,
	.gpr2 = 0,
	.gpr3 = 0,
	.gpr4 = 0,
	.gpr5 = 0,
	.gpr6 = 0,
	.gpr7 = 0,
	.gpr8 = 0,
	.gpr9 = 0,
	.gpr10 = 0,
	.gpr11 = 0,
	.gpr12 = 0,
	.gpr13 = 0,
	.gpr14 = 0,
	.gpr15 = 0,
	.gpr16 = 0,
	.gpr17 = 0,
	.gpr18 = 0,
	.gpr19 = 0,
	.gpr20 = 0,
	.gpr21 = 0,
	.gpr22 = 0,
	.gpr23 = 0,
	.gpr24 = 0,
	.gpr25 = 0,
	.gpr26 = 0,
	.gpr27 = 0,
	.gpr28 = 0,
	.gpr29 = 0,
	.gpr30 = 0,
	.gpr31 = 0,
};
#elif defined(INTEL)
#	if __x86_64__
registers_state_t injected_reg_state = {
	.rax=0,
	.rbx=0,
	.rcx=0,
	.rdx=0,
	.rsi=0,
	.rdi=0,
	.r8=0,
	.r9=0,
	.r10=0,
	.r11=0,
	.r12=0,
	.r13=0,
	.r14=0,
	.r15=0,
	.rbp=0,
	.rsp=0,
};
#	else
registers_state_t injected_reg_state = {
	.eax=0,
	.ebx=0,
	.ecx=0,
	.edx=0,
	.esi=0,
	.edi=0,
	.ebp=0,
	.esp=0,
};
#	endif
#endif
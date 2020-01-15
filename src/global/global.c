#include "global.h"

global_config = {
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

global_search_mode = TUNNEL;
current_instruction = {};

#ifdef LINUX
stack = { 0 };
ss = { .ss_size = SIGSTKSZ, .ss_sp = stack, };
#endif

packet_buffer = NULL;
packet = NULL;

#if __x86_64__
injected_reg_state = {
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
#else
injected_reg_state = {
	.eax=0,
	.ebx=0,
	.ecx=0,
	.edx=0,
	.esi=0,
	.edi=0,
	.ebp=0,
	.esp=0,
};
#endif
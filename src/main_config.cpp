//
// Created by student on 2/11/20.
//

#include "ConfigManager/Definitions.hpp"
#include "InstructionManager/Definitions.hpp"

// CONFIG FOR APPLICATION
Config global_config =
{
        false,
        0,
        4,
        0,
        0,
        false,
        1,
        false,
        0,
        false,
        true,
};

// REGISTERS STATE FOR PROCESSOR
RegState injected_reg_state =
{
#if PROCESSOR == POWER_PC
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
#elif PROCESSOR == INTEL
#	if __x86_64__
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
#	else
	.eax=0,
	.ebx=0,
	.ecx=0,
	.edx=0,
	.esi=0,
	.edi=0,
	.ebp=0,
	.esp=0,
#	endif
#endif
};
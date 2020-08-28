//
// Created by student on 2/11/20.
//

#include "ConfigManager/Definitions.hpp"

IgnoredOpcode opcode_blacklist[MAX_BLACKLISTED_OPCODES] =
{
#if PROCESSOR == POWER_PC
        // TODO: POWERPC
#elif PROCESSOR == INTEL
        { "\x0f\x34", "sysenter" },
        { "\x0f\xa1", "pop fs" },
        { "\x0f\xa9", "pop gs" },
        { "\x8e", "mov seg" },
        { "\xc8", "enter" },
#   if !__x86_64__
/* vex in 64 (though still can be vex in 32...) */
	{ "\xc5", "lds" },
	{ "\xc4", "les" },
#   endif
        { "\x0f\xb2", "lss" },
        { "\x0f\xb4", "lfs" },
        { "\x0f\xb5", "lgs" },
#   if __x86_64__
        /* 64 bit only - intel "discourages" using this without a rex* prefix, and
         * so capstone doesn't parse it */
        { "\x63", "movsxd" },
#   endif
        /* segfaulting with various "mov sp" (always sp) in random synthesizing, too
         * tired to figure out why: 66 bc7453 */
        { "\xbc", "mov sp" },
        /* segfaulting with "shr sp, 1" (always sp) in random synthesizing, too tired to
         * figure out why: 66 d1ec */
        /* haven't observed but assuming "shl sp, 1" and "sar sp, 1" fault as well */
        { "\xd1\xec", "shr sp, 1" },
        { "\xd1\xe4", "shl sp, 1" },
        { "\xd1\xfc", "sar sp, 1" },
        /* same with "rcr sp, 1", assuming same for rcl, ror, rol */
        { "\xd1\xdc", "rcr sp, 1" },
        { "\xd1\xd4", "rcl sp, 1" },
        { "\xd1\xcc", "ror sp, 1" },
        { "\xd1\xc4", "rol sp, 1" },
        /* same with lea sp */
        { "\x8d\xa2", "lea sp" },
        /* i guess these are because if you shift esp, you wind up way outside your
         * address space; if you shift sp, just a little, you stay in and crash */
        /* unable to resolve a constant length for xbegin, causes tunnel to stall */
        { "\xc7\xf8", "xbegin" },
        /* int 80 will obviously cause issues */
        { "\xcd\x80", "int 0x80" },
        /* as will syscall */
        { "\x0f\x05", "syscall" },
        /* ud2 is an undefined opcode, and messes up a length differential search
         * b/c of the fault it throws */
        { "\x0f\xb9", "ud2" },
#endif

        { NULL, NULL }
};

IgnoredPrefix prefix_blacklist[MAX_BLACKLISTED_PREFIXES] =
{
#if PROCESSOR == POWER_PC
        // TODO: POWERPC
#elif PROCESSOR == INTEL
#   if !__x86_64__
/* avoid overwriting tls or something in 32 bit code */
	{ "\x65", "gs" },
#   endif
#endif

        { NULL, NULL }
};
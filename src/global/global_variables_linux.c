#include "global.h"

#ifdef LINUX
char stack[SIGSTKSZ] = { 0 };
stack_t ss = { .ss_size = SIGSTKSZ, .ss_sp = stack, };

mcontext_t fault_context = {};

pthread_mutex_t* pool_mutex = NULL;
pthread_mutex_t* output_mutex = NULL;
pthread_mutexattr_t mutex_attr = {};

#   if USE_CAPSTONE
csh      capstone_handle;
cs_insn *capstone_insn = NULL;
disassembly_info_t disas_info = {};
#   endif

#endif
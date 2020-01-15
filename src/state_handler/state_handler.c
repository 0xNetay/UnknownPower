#include "state_handler.h"

void state_handler(int signum, siginfo_t* si, void* p)
{
	fault_context=((ucontext_t*)p)->uc_mcontext;
	((ucontext_t*)p)->uc_mcontext.gregs[IP]+=UD2_SIZE;
}

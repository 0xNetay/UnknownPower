#include "fault_handler.h"

void fault_handler(int signum, siginfo_t* si, void* p)
{
	int insn_length;
	ucontext_t* uc=(ucontext_t*)p;
	int preamble_length=(&preamble_end-&preamble_start);

	if (!USE_TF) { preamble_length=0; }

	/* make an initial estimate on the instruction length from the fault address */
	insn_length=
		(uintptr_t)uc->uc_mcontext.gregs[IP]-(uintptr_t)packet-preamble_length;

	if (insn_length<0) {
		insn_length=JMP_LENGTH;
	}
	else if (insn_length>MAX_INSN_LENGTH) {
		insn_length=JMP_LENGTH;
	}

	result=(result_t){
		1,
		insn_length,
		signum,
		si->si_code,
		(signum==SIGSEGV||signum==SIGBUS)?(uint32_t)(uintptr_t)si->si_addr:(uint32_t)-1
	};

	memcpy(uc->uc_mcontext.gregs, fault_context.gregs, sizeof(fault_context.gregs));
	uc->uc_mcontext.gregs[IP]=(uintptr_t)&resume;
	uc->uc_mcontext.gregs[REG_EFL]&=~TF;
}

void configure_sig_handler(void (*handler)(int, siginfo_t*, void*))
{
	struct sigaction s;

	s.sa_sigaction=handler;
	s.sa_flags=SA_SIGINFO|SA_ONSTACK;

	sigfillset(&s.sa_mask);

	sigaction(SIGILL,  &s, NULL);
	sigaction(SIGSEGV, &s, NULL);
	sigaction(SIGFPE,  &s, NULL);
	sigaction(SIGBUS,  &s, NULL);
	sigaction(SIGTRAP, &s, NULL);
}
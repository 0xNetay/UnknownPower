#include "injector.h"

void inject(int insn_size)
{
	/* could probably fork here to avoid risk of destroying the controlling process */
	/* only really comes up in random injection, just roll the dice for now */

	int i;
	int preamble_length=(&preamble_end-&preamble_start);
	static bool have_state=false;

	if (!USE_TF) { preamble_length=0; }

	packet=packet_buffer+PAGE_SIZE-insn_size-preamble_length;

	/* optimization - don't bother to write protect page */
	//	assert(!mprotect(packet_buffer,PAGE_SIZE,PROT_READ|PROT_WRITE|PROT_EXEC));
	for (i=0; i<preamble_length; i++) {
		((char*)packet)[i]=((char*)&preamble_start)[i];
	}
	for (i=0; i<MAX_INSN_LENGTH && i<insn_size; i++) {
		((char*)packet)[i+preamble_length]=inj.i.bytes[i];
	}
	//	assert(!mprotect(packet_buffer,PAGE_SIZE,PROT_READ|PROT_EXEC));

	if (config.enable_null_access) {
		/* without this we need to blacklist any instruction that modifies esp */
		void* p=NULL; /* suppress warning */
		memset(p, 0, PAGE_SIZE);
	}

	dummy_stack.dummy_stack_lo[0]=0;

	if (!have_state) {
		/* optimization: only get state first time */
		have_state=true;
		configure_sig_handler(state_handler);
		__asm__ __volatile__ ("ud2\n");
	}

	configure_sig_handler(fault_handler);

#if __x86_64__
	__asm__ __volatile__ ("\
			mov %[rax], %%rax \n\
			mov %[rbx], %%rbx \n\
			mov %[rcx], %%rcx \n\
			mov %[rdx], %%rdx \n\
			mov %[rsi], %%rsi \n\
			mov %[rdi], %%rdi \n\
			mov %[r8],  %%r8  \n\
			mov %[r9],  %%r9  \n\
			mov %[r10], %%r10 \n\
			mov %[r11], %%r11 \n\
			mov %[r12], %%r12 \n\
			mov %[r13], %%r13 \n\
			mov %[r14], %%r14 \n\
			mov %[r15], %%r15 \n\
			mov %[rbp], %%rbp \n\
			mov %[rsp], %%rsp \n\
			jmp *%[packet]    \n\
			"
			: /* no output */
			: [rax]"m"(inject_state.rax),
			  [rbx]"m"(inject_state.rbx),
			  [rcx]"m"(inject_state.rcx),
			  [rdx]"m"(inject_state.rdx),
			  [rsi]"m"(inject_state.rsi),
			  [rdi]"m"(inject_state.rdi),
			  [r8]"m"(inject_state.r8),
			  [r9]"m"(inject_state.r9),
			  [r10]"m"(inject_state.r10),
			  [r11]"m"(inject_state.r11),
			  [r12]"m"(inject_state.r12),
			  [r13]"m"(inject_state.r13),
			  [r14]"m"(inject_state.r14),
			  [r15]"m"(inject_state.r15),
			  [rbp]"m"(inject_state.rbp),
			  [rsp]"i"(&dummy_stack.dummy_stack_lo),
			  [packet]"m"(packet)
			);
#else
	__asm__ __volatile__ ("\
			mov %[eax], %%eax \n\
			mov %[ebx], %%ebx \n\
			mov %[ecx], %%ecx \n\
			mov %[edx], %%edx \n\
			mov %[esi], %%esi \n\
			mov %[edi], %%edi \n\
			mov %[ebp], %%ebp \n\
			mov %[esp], %%esp \n\
			jmp *%[packet]    \n\
			"
			:
			:
			[eax]"m"(inject_state.eax),
			[ebx]"m"(inject_state.ebx),
			[ecx]"m"(inject_state.ecx),
			[edx]"m"(inject_state.edx),
			[esi]"m"(inject_state.esi),
			[edi]"m"(inject_state.edi),
			[ebp]"m"(inject_state.ebp),
			[esp]"i"(&dummy_stack.dummy_stack_lo),
			[packet]"m"(packet)
			);
#endif

	__asm__ __volatile__ ("\
			.global resume   \n\
			resume:          \n\
			"
			);
	;
}
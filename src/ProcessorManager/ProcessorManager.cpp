//
// Created by student on 2/11/20.
//

#include "ProcessorManager.hpp"
#include "ConfigManager/ConfigManager.hpp"
#include "OutputManager/OutputManager.hpp"

bool ProcessorManager::_is_first = true;
mcontext_t ProcessorManager::_fault_context = {};

void* ProcessorManager::_packet_buffer = nullptr;
char* ProcessorManager::_packet = nullptr;

extern char debug;
extern char resume;
extern char preamble_start;
extern char preamble_end;
DummyStack dummy_stack = {};

void ProcessorManager::InjectInstructionToProcessor(const Instruction& instruction)
{
    // Length of the preamble code section
    size_t preamble_length = (&preamble_end - &preamble_start);

    // Find which is smaller, max length or given instruction
    size_t minimum = instruction.length <= MAX_INSTRUCTION_LENGTH ? instruction.length : MAX_INSTRUCTION_LENGTH;

#if !USE_TF
    preamble_length = 0;
#endif

    // Takes a piece of the memory space to be used by the preamble and injected instruction
    // Note that this location is at the end of the executable region within our buffer (first page is executable, second is not)
    ProcessorManager::_packet =
        reinterpret_cast<char*>(ProcessorManager::_packet_buffer) + PAGE_SIZE - instruction.length - preamble_length;

    // Copy the preamble code and right after it the instruction byte code into the packet (address that will be ran)
    memcpy(ProcessorManager::_packet, &preamble_start, preamble_length);
    memcpy(ProcessorManager::_packet + preamble_length, instruction.bytes.data(), minimum);

    // Without this we need to blacklist any instruction that modifies esp
    if (ConfigManager::Instance().GetConfig().enable_null_access)
    {
        void* p = NULL;
        memset(p, 0, PAGE_SIZE);
    }

    dummy_stack.dummy_stack_lo[0] = 0;

    // Generates undefined instruction for the first time before any injection, to get the original fault context
    if (ProcessorManager::_is_first)
    {
        ProcessorManager::ConfigureSignalHandler();
        ProcessorManager::_is_first = false;

        // Create a first fault with undefined instruction
#if PROCESSOR == POWER_PC
        // TODO: POWERPC
#elif PROCESSOR == INTEL
        __asm__ __volatile__ ("ud2\n");
#endif
    }

    // Configure our signal handler
    ProcessorManager::ConfigureSignalHandler();

    RegState inject_state = ConfigManager::Instance().GetRegState();

    // Inject all the registers with their default state (from config manager
    // Configure the stack pointer with our dummy stack
    // Jump to packet - our preamble + injected opcode
#if PROCESSOR == POWER_PC
    // TODO: POWERPC
#elif PROCESSOR == INTEL
#   if __x86_64__
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
            :
            :   [rax]"m"(inject_state.rax),
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
                [packet]"m"(_packet)
    );
#   else
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
			:   [eax]"m"(inject_state.eax),
			    [ebx]"m"(inject_state.ebx),
			    [ecx]"m"(inject_state.ecx),
			    [edx]"m"(inject_state.edx),
			    [esi]"m"(inject_state.esi),
			    [edi]"m"(inject_state.edi),
			    [ebp]"m"(inject_state.ebp),
			    [esp]"i"(&dummy_stack->dummy_stack_lo),
			    [packet]"m"(_packet)
			);
#   endif
#endif

    // Set that resume label will point to here, so we will return to the end of this function after finishing the fault handler
    __asm__ __volatile__ ("\
			.global resume   \n\
			resume:          \n\
			"
    );
    ;
}

bool ProcessorManager::PinCore()
{
    if (ConfigManager::Instance().GetConfig().force_core)
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(ConfigManager::Instance().GetConfig().core_count,&mask);
        if (sched_setaffinity(0, sizeof(mask), &mask))
        {
            printf("error: failed to set cpu\n");
            return false;
        }
    }

    return true;
}

void ProcessorManager::ConfigureSignalHandler()
{
    struct sigaction signal_action = {};

    if (ProcessorManager::_is_first)
    {
        signal_action.sa_sigaction = ProcessorManager::FirstStateHandler;
    }
    else
    {
        signal_action.sa_sigaction = ProcessorManager::FaultHandler;
    }

    signal_action.sa_flags = SA_SIGINFO | SA_ONSTACK;

    sigfillset(&signal_action.sa_mask);

    sigaction(SIGILL,  &signal_action, NULL);
    sigaction(SIGSEGV, &signal_action, NULL);
    sigaction(SIGFPE,  &signal_action, NULL);
    sigaction(SIGBUS,  &signal_action, NULL);
    sigaction(SIGTRAP, &signal_action, NULL);
}

void ProcessorManager::FirstStateHandler(int signal_number, siginfo_t* signal_info, void* context)
{
    ucontext_t* signal_context = reinterpret_cast<ucontext_t*>(context);

    ProcessorManager::_fault_context = signal_context->uc_mcontext;
    signal_context->uc_mcontext.gregs[IP] += UNDEFINED_INSTRUCTION_LENGTH;
}

void ProcessorManager::FaultHandler(int signal_number, siginfo_t* signal_info, void* context)
{
    // TODO: Check what to do with POWERPC

    ucontext_t* signal_context = reinterpret_cast<ucontext_t*>(context);
    size_t preamble_length = (&preamble_end - &preamble_start);

#if !USE_TF
    preamble_length = 0;
#endif

    // Make an initial estimate on the instruction length from the fault address
    uintptr_t ip_before_signal = signal_context->uc_mcontext.gregs[IP];
    int instruction_length = ip_before_signal - uintptr_t(ProcessorManager::_packet) - preamble_length;

    // If something is wrong with the length, we assume it is the length of a jump instruction, or something that couldn't be determined
    if (instruction_length < 0 || instruction_length > MAX_INSTRUCTION_LENGTH)
    {
        instruction_length = JUMP_OR_UNDETERMINED_INSTRUCTION_LENGTH;
    }

    // Set the result with our findings
    Result* result_ptr = &OutputManager::Instance().GetMutableResult();
    result_ptr->valid = 1;
    result_ptr->length = instruction_length;
    result_ptr->signum = signal_number;
    result_ptr->si_code = signal_info->si_code;
    result_ptr->address =
        (signal_number == SIGSEGV || signal_number==SIGBUS) ?
        static_cast<uint32_t>((uintptr_t)signal_info->si_addr) :
        static_cast<uint32_t>(-1);

    // Set the GREGS to the one we got from our initiated undefined instruction on first tine
    memcpy(signal_context->uc_mcontext.gregs, ProcessorManager::_fault_context.gregs, sizeof(ProcessorManager::_fault_context.gregs));

    // Set the IP of our return from the interrupt to the resume label
    signal_context->uc_mcontext.gregs[IP] = (uintptr_t)&resume;
    signal_context->uc_mcontext.gregs[REG_EFL] &= ~TF;
}

void ProcessorManager::Preamble()
{
#if PROCESSOR == POWER_PC
    // TODO
#elif PROCESSOR == INTEL
#   if __x86_64__
    __asm__ __volatile__ ("\
			.global preamble_start                    \n\
			preamble_start:                           \n\
			pushfq                                    \n\
			orq %0, (%%rsp)                           \n\
			popfq                                     \n\
			.global preamble_end                      \n\
			preamble_end:                             \n\
			"
			:
			:"i"(TF)
			);
#   else
	__asm__ __volatile__ ("\
			.global preamble_start                    \n\
			preamble_start:                           \n\
			pushfl                                    \n\
			orl %0, (%%esp)                           \n\
			popfl                                     \n\
			.global preamble_end                      \n\
			preamble_end:                             \n\
			"
			:
			:"i"(TF)
			);
#   endif
#endif
}
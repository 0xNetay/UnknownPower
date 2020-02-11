//
// Created by student on 2/11/20.
//

#include <src/OutputManager/OutputManager.hpp>
#include "ProcessorManager.hpp"
#include "ConfigManager/ConfigManager.hpp"
#include "OutputManager/OutputManager.hpp"

bool ProcessorManager::_is_first = true;
DummyStack ProcessorManager::_dummy_stack = {};
mcontext_t ProcessorManager::_fault_context = {};

void* ProcessorManager::_packet_buffer = nullptr;
char* ProcessorManager::_packet = nullptr;

char ProcessorManager::_debug;
char ProcessorManager::_resume;
char ProcessorManager::_preamble_start;
char ProcessorManager::_preamble_end;

void ProcessorManager::InjectInstructionToProcessor(int instruction_length)
{
    // TODO
}

void ProcessorManager::PinCore()
{
    if (ConfigManager::Instance().GetConfig().force_core)
    {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(ConfigManager::Instance().GetConfig().core_count,&mask);
        if (sched_setaffinity(0, sizeof(mask), &mask))
        {
            printf("error: failed to set cpu\n");
            exit(1);
        }
    }
}

void ProcessorManager::ConfigureSignalHandler()
{
    struct sigaction signal_action = {};

    if (_is_first)
    {
        signal_action.sa_sigaction = ProcessorManager::FirstStateHandler;
    }
    else
    {
        signal_action.sa_sigaction = ProcessorManager::FaultHandler;
    }

    signal_action.sa_flags=SA_SIGINFO|SA_ONSTACK;

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
    signal_context->uc_mcontext.gregs[IP] += UD2_SIZE;
}

void ProcessorManager::FaultHandler(int signal_number, siginfo_t* signal_info, void* context)
{
    // TODO: Check what to do with POWERPC

    size_t instruction_length;
    ucontext_t* signal_context = reinterpret_cast<ucontext_t*>(context);
    size_t preamble_length = (&_preamble_end - &_preamble_start);

#if !USE_TF
    preamble_length=0;
#endif

    /* make an initial estimate on the instruction length from the fault address */
    instruction_length = (uintptr_t)signal_context->uc_mcontext.gregs[IP] - (uintptr_t)_packet - preamble_length;

    if (instruction_length < 0)
    {
        instruction_length = JUMP_OR_UNDETERMINED_INSTRUCTION_LENGTH;
    }
    else if (instruction_length > MAX_INSTRUCTION_LENGTH)
    {
        instruction_length = JUMP_OR_UNDETERMINED_INSTRUCTION_LENGTH;
    }

    Result* result_ptr = &OutputManager::Instance().GetMutableResult();
    result_ptr->valid = 1;
    result_ptr->length = instruction_length;
    result_ptr->signum = signal_number;
    result_ptr->si_code = signal_info->si_code;
    result_ptr->address =
        (signal_number == SIGSEGV || signal_number==SIGBUS) ?
        static_cast<uint32_t>((uintptr_t)signal_info->si_addr) :
        static_cast<uint32_t>(-1);

    memcpy(signal_context->uc_mcontext.gregs, _fault_context.gregs, sizeof(_fault_context.gregs));
    signal_context->uc_mcontext.gregs[IP] = (uintptr_t)&_resume;
    signal_context->uc_mcontext.gregs[REG_EFL] &= ~TF;
}

void ProcessorManager::Preamble()
{
#ifdef POWER_PC
    // TODO
#elif __x86_64__
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
#else
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
#endif
}
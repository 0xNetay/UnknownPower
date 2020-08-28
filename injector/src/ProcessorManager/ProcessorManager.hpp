//
// Created by student on 2/11/20.
//

#ifndef UNKNOWNPOWER_PROCESSORMANAGER_HPP
#define UNKNOWNPOWER_PROCESSORMANAGER_HPP

#include "Definitions.hpp"
#include "InstructionManager/Definitions.hpp"

class ProcessorManager
{
public:
    /* Setups registers and fault handler, and then gives (injects) the instruction to the processor by jumping to the preamble */
    static void InjectInstructionToProcessor(const Instruction& instruction);

    /* Configure the usage of different CPU cores */
    static bool PinCore();

    /* Get a pointer to the memory space (pointer) the processor manager uses to run code from */
    inline static void** GetMutablePacketBuffer() { return &_packet_buffer; }

    /* Get a pointer to a pointer which leads to the instruction address within the memory space of the processor manager */
    inline static char** GetMutablePacket() { return &_packet; }

private:
    /* Static class, no instances are allowed */
    inline ProcessorManager() = default;

    /* Constructs signal handler, with the wanted function (first time and run time) and then setup the handler to the possible fault signals */
    static void ConfigureSignalHandler();

    /* First time signal handler to set up fault context for the manager */
    static void FirstStateHandler(int signal_number, siginfo_t* signal_info, void* context);

    /* Handling our injected instruction fault, checking length and setting the result with the information found*/
    static void FaultHandler(int signal_number, siginfo_t* signal_info, void* context);

    /* An assembly code that is injected into the packet space, before the injected instruction */
    static void Preamble();

    /* Outsource */
    static void* _packet_buffer;        // Memory space for the processor manager
    static char* _packet;               // Where the preamble + injected instruction sits, within the memory space (buffer)

    /* Misc */
    static bool _is_first;              // Flag to indicate this is the first injection, thus using FirstStateHandler then
    static mcontext_t _fault_context;   // Keeps the original fault context from an initiated fault, to be injected after running arbitrary byte code
};

#endif //UNKNOWNPOWER_PROCESSORMANAGER_HPP

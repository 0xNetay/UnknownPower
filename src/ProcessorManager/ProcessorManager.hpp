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
    /* Setups registers and fault handler, and then gives the instruction to the processor */
    static void InjectInstructionToProcessor(const Instruction& instruction);

    static void PinCore();

    inline static void** GetMutablePacketBuffer() { return &_packet_buffer; }
    inline static char** GetMutablePacket() { return &_packet; }

private:
    inline ProcessorManager() = default;

    static void ConfigureSignalHandler();
    static void FirstStateHandler(int signal_number, siginfo_t* signal_info, void* context);
    static void FaultHandler(int signal_number, siginfo_t* signal_info, void* context);

    static void Preamble();

    static bool _is_first;
    static mcontext_t _fault_context;

    static void* _packet_buffer;
    static char* _packet;
};

#endif //UNKNOWNPOWER_PROCESSORMANAGER_HPP

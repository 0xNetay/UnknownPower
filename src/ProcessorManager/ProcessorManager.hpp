//
// Created by student on 2/11/20.
//

#ifndef UNKNOWNPOWER_PROCESSORMANAGER_HPP
#define UNKNOWNPOWER_PROCESSORMANAGER_HPP

#include "Definitions.hpp"

class ProcessorManager
{
public:
    /* Setups registers and fault handler, and then gives the instruction to the processor */
    static void InjectInstructionToProcessor(int instruction_length);

    static void PinCore();

    inline static void* GetMutablePacketBuffer() { return _packet_buffer }
    inline static char* GetMutablePacket() { return _packet }

private:
    inline ProcessorManager() = default;

    static void ConfigureSignalHandler();
    static void FirstStateHandler(int signal_number, siginfo_t* signal_info, void* context);
    static void FaultHandler(int signal_number, siginfo_t* signal_info, void* context);

    static void Preamble();

    static bool _is_first;
    static DummyStack _dummy_stack;
    static mcontext_t _fault_context;

    static void* _packet_buffer;
    static char* _packet;

    static char _debug;
    static char _resume;
    static char _preamble_start;
    static char _preamble_end;
};

#endif //UNKNOWNPOWER_PROCESSORMANAGER_HPP

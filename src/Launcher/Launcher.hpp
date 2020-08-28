//
// Created by student on 2/29/20.
//

#include "General.hpp"

#ifndef UNKNOWNPOWER_LAUNCHER_HPP
#define UNKNOWNPOWER_LAUNCHER_HPP

#include "ConfigManager/ConfigManager.hpp"
#include "OutputManager/OutputManager.hpp"
#include "InstructionManager/InstructionManager.hpp"
#include "ProcessorManager/ProcessorManager.hpp"

class Launcher
{
public:
    /* Creates a new launcher instance */
    Launcher(int argc, char** argv);

    /* Initializes the mutexes, working modes, processor cores and stack, builder ranges and establishes user-made configurations */
    bool Init();

    /* Creates multiple processes before main loop, which builds ranges and instructions and inject them one after the other until reaches the end */
    bool Run();

    /* Flushes any remaining output waiting in buffers, free mutexes, ranges and ProcessorManager's space (packet buffer) */
    bool Close();

private:
    /* User-Made configurations */
    bool Configure();

    /* Prints help for available config arguments */
    static void PrintHelp();

    /* Prints how to use the program */
    static void PrintUsage();

    /* User Config */
    int _argc;
    char** _argv;

    /* System Mutexes */
    pthread_mutex_t* _pool_mutex = nullptr;         // Mutex for the instruction building, so each process has time to create needed things within the memory pool
    pthread_mutex_t* _output_mutex = nullptr;       // Mutex for synchronize output printing
    pthread_mutexattr_t _mutex_attr = {};

    InstructionManager _instruction_manager;        // Holding instance of the instruction manager

#if USE_CAPSTONE
    /* Capstone */
    csh      _capstone_handle = {};
    cs_insn* _capstone_insn = nullptr;
    DisassemblyInfo _disassembly_info = {};
#endif

    /* Processor Pointers */
    void* _packet_buffer_unaligned = nullptr;
    void* _null_p = nullptr;
};

#endif //UNKNOWNPOWER_LAUNCHER_HPP

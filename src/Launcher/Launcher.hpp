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
    Launcher(int argc, char** argv);

    bool Init();
    void Run();
    void Close();

private:
    bool Configure();

    static void PrintHelp();

    static void PrintUsage();

    int _argc;
    char** _argv;

    pthread_mutex_t* _pool_mutex = nullptr;
    pthread_mutex_t* _output_mutex = nullptr;
    pthread_mutexattr_t _mutex_attr = {};

    InstructionManager _instruction_manager;

#if USE_CAPSTONE
    csh      _capstone_handle = {};
    cs_insn* _capstone_insn = nullptr;
    DisassemblyInfo _disassembly_info = {};
#endif

    void* _packet_buffer_unaligned = nullptr;
    void* _null_p = nullptr;
};

#endif //UNKNOWNPOWER_LAUNCHER_HPP

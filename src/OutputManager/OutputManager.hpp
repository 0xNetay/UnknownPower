//
// Created by student on 2/8/20.
//

#ifndef UNKNOWNPOWER_OUTPUTMANAGER_HPP
#define UNKNOWNPOWER_OUTPUTMANAGER_HPP

#include "Definitions.hpp"
#include "InstructionManager/Definitions.hpp"

class OutputManager
{
public:
    inline static OutputManager& Instance()
    {
        static OutputManager OutputManager;
        return OutputManager;
    }

    inline const Result& GetImmutableResult() { return _current_result; }
    inline Result& GetMutableResult() { return _current_result; }

    inline void SetOutputMode(const OutputMode& output_mode) { _output_mode = output_mode; }
    inline const OutputMode& GetOutputMode() { return _output_mode; }

    inline void SetOutputMutex(pthread_mutex_t* output_mutex) { _output_mutex = output_mutex; }
    inline const pthread_mutex_t* GetImmutableOutputMutex() { return _output_mutex; }
    inline pthread_mutex_t* GetMutableOutputMutex() { return _output_mutex; }

    inline void SetPacketBuffer(void* packet_buffer) { _packet_buffer = packet_buffer; }
    inline const void* GetImmutablePacketBuffer() { return _packet_buffer; }
    inline void* GetMutablePacketBuffer() { return _packet_buffer; }

#if USE_CAPSTONE
    inline void SetCapstoneHandler(const csh& capstone_handle) { _capstone_handle = capstone_handle; }
    inline const csh& GetImmutableCapstoneHandler() { return _capstone_handle; }
    inline csh& GetMutableCapstoneHandler() { return _capstone_handle; }

    inline void SetCapstoneInstructions(cs_insn* capstone_insn) { _capstone_insn = capstone_insn; }
    inline const cs_insn* GetImmutableCapstoneInstructions() { return _capstone_insn; }
    inline cs_insn* GetMutableCapstoneInstructions() { return _capstone_insn; }

    inline const DisassemblyInfo& GetImmutableDisassemblyInfo() { return _disassembly_info; }
    inline DisassemblyInfo& GetMutableDisassemblyInfo() { return _disassembly_info; }
#endif

    void PrintInMcToOutput(const Instruction& instruction, FILE* output_file);
    void PrintInMcToOutput(const Instruction& instruction, size_t instruction_length, FILE* output_file);
    void GiveResultToOutput(const Instruction& instruction, BuildMode build_mode, FILE* output_file);

    void SyncPrintFormat(FILE* output_file, const char *format, ...);
    void SyncWriteBuffer(const void* output_buffer, size_t single_element_size, size_t elements_count_to_write, FILE* output_file);
    void SyncFlushOutput(FILE* output_file, bool force);

    void PrintInstructionToOutput(const Instruction& instruction, FILE* output_file);
    void PrintRangeToOutput(const InstructionRange &range, FILE* output_file);
#if USE_CAPSTONE
    int PrintInstructionInAsmToOutput(const Instruction& instruction, FILE* output_file);
#endif

    void Tick(const Instruction& instruction, BuildMode build_mode);

private:
    inline OutputManager() = default;

    Result _current_result = {};
    OutputMode _output_mode = OutputMode::Text;

#if USE_CAPSTONE
    csh      _capstone_handle = {};
    cs_insn* _capstone_insn = nullptr;
    DisassemblyInfo _disassembly_info = {};
#endif

    static constexpr size_t LINE_BUFFER_SIZE = 256;
    static constexpr size_t BUFFER_LINES = 16;
    static constexpr size_t SYNC_LINES_STDOUT = BUFFER_LINES;
    static constexpr size_t SYNC_LINES_STDERR = BUFFER_LINES;

    char    _stdout_buffer[LINE_BUFFER_SIZE*BUFFER_LINES] = { 0 };
    char*   _stdout_buffer_pos = &_stdout_buffer[0];
    size_t  _stdout_sync_counter = 0;
    char    _stderr_buffer[LINE_BUFFER_SIZE*BUFFER_LINES] = { 0 };
    char*   _stderr_buffer_pos = &_stderr_buffer[0];
    size_t  _stderr_sync_counter = 0;
    size_t  _expected_length = 0;

    pthread_mutex_t* _output_mutex = nullptr;
    void*            _packet_buffer = nullptr;
};

#endif //UNKNOWNPOWER_OUTPUTMANAGER_HPP

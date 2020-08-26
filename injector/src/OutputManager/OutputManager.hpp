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
    /* One instance for all across the system */
    inline static OutputManager& Instance()
    {
        static OutputManager OutputManager;
        return OutputManager;
    }

    /* Access to the Result that will be printed to output */
    inline const Result& GetImmutableResult() { return _current_result; }
    inline Result& GetMutableResult() { return _current_result; }

    /* Access to the output mode the manager works in */
    inline void SetOutputMode(const OutputMode& output_mode) { _output_mode = output_mode; }
    inline const OutputMode& GetOutputMode() { return _output_mode; }

    /* Access to the output mutex the manager keeps to prevent races when using multiple processes */
    inline void SetOutputMutex(pthread_mutex_t** output_mutex) { _output_mutex = output_mutex; }
    inline pthread_mutex_t** GetMutableOutputMutex() { return _output_mutex; }

    /* Access to the packet buffer pointer, the output manager has access to the ProcessorManager's memory space, to print instructions and other info */
    inline void SetPacketBuffer(void** packet_buffer) { _packet_buffer = packet_buffer; }
    inline void** GetMutablePacketBuffer() { return _packet_buffer; }

#if USE_CAPSTONE
    /* Access to the capstone handler to be set (startup) and used (runtime) */
    inline void SetCapstoneHandler(const csh& capstone_handle) { _capstone_handle = capstone_handle; }
    inline const csh& GetImmutableCapstoneHandler() { return _capstone_handle; }
    inline csh& GetMutableCapstoneHandler() { return _capstone_handle; }

    /* Access to the capstone instruction holder */
    inline void SetCapstoneInstructions(cs_insn* capstone_insn) { _capstone_insn = capstone_insn; }
    inline const cs_insn* GetImmutableCapstoneInstructions() { return _capstone_insn; }
    inline cs_insn* GetMutableCapstoneInstructions() { return _capstone_insn; }

    /* Access to the dissasmebly information we have for raw output */
    inline const DisassemblyInfo& GetImmutableDisassemblyInfo() { return _disassembly_info; }
    inline DisassemblyInfo& GetMutableDisassemblyInfo() { return _disassembly_info; }
#endif

    /* Printers that print the instruction hex byte code in a readable way */
    void PrintInMcToOutput(const Instruction& instruction, FILE* output_file);
    void PrintInMcToOutput(const Instruction& instruction, size_t instruction_length, FILE* output_file);

    /* Print the result from last injection in a readable way */
    void GiveResultToOutput(const Instruction& instruction, BuildMode build_mode, FILE* output_file);

    /* Write to our internal buffers in a print format */
    void SyncPrintFormat(FILE* output_file, const char *format, ...);

    /* Write to our internal buffers */
    void SyncWriteBuffer(const void* output_buffer, size_t single_element_size, size_t elements_count_to_write, FILE* output_file);

    /* Flush our internal buffers to the correspond output file if reached maximnm capacity in internal buffers or we are forced to */
    void SyncFlushOutput(FILE* output_file, bool force);

    /* Printers that print the instruction / range / ASM hex byte code */
    void PrintInstructionToOutput(const Instruction& instruction, FILE* output_file);
    void PrintRangeToOutput(const InstructionRange &range, FILE* output_file);
#if USE_CAPSTONE
    int PrintInstructionInAsmToOutput(const Instruction& instruction, FILE* output_file);
#endif

    /* Printing information every opcode iteration in an organized way (besides any other requested printing)  */
    void Tick(const Instruction& instruction, BuildMode build_mode);

private:
    /* Singleton class, no instances are allowed */
    inline OutputManager() = default;

    /* Outsource */
    pthread_mutex_t** _output_mutex = nullptr;      // Prevent multiple processes to flush to output
    void**            _packet_buffer = nullptr;     // Access to the ProcessorManager memory space

    /* Result */
    Result _current_result = {};                    // Result of last injection
    OutputMode _output_mode = OutputMode::Text;     // How we write result to output

#if USE_CAPSTONE
    /* Capstone */
    csh      _capstone_handle = {};                 // Handler for cap
    cs_insn* _capstone_insn = nullptr;              // Instruction holder for cap
    DisassemblyInfo _disassembly_info = {};         // Disassembly info of the instruction
#endif

    /* Buffer Constants */
    static constexpr size_t LINE_BUFFER_SIZE = 256;
    static constexpr size_t BUFFER_LINES = 16;
    static constexpr size_t SYNC_LINES_STDOUT = BUFFER_LINES;
    static constexpr size_t SYNC_LINES_STDERR = BUFFER_LINES;

    /* Printing Buffers */
    char    _stdout_buffer[LINE_BUFFER_SIZE*BUFFER_LINES] = { 0 };
    char*   _stdout_buffer_pos = &_stdout_buffer[0];
    size_t  _stdout_sync_counter = 0;
    char    _stderr_buffer[LINE_BUFFER_SIZE*BUFFER_LINES] = { 0 };
    char*   _stderr_buffer_pos = &_stderr_buffer[0];
    size_t  _stderr_sync_counter = 0;
    size_t  _expected_length = 0;
};

#endif //UNKNOWNPOWER_OUTPUTMANAGER_HPP

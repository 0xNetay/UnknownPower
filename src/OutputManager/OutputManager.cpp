//
// Created by student on 2/8/20.
//

#include "OutputManager.hpp"
#include "ConfigManager/ConfigManager.hpp"
#include "InstructionManager/InstructionManager.hpp"

void OutputManager::PrintInMcToOutput(const Instruction& instruction, size_t instruction_length, FILE* output_file)
{
    bool prefix_flag = false;
    
    if (!InstructionManager::IsPrefix(instruction.bytes[0]))
    {
        this->SyncPrintFormat(output_file, " ");
        prefix_flag = true;
    }
    
    for (size_t i=0; (i < instruction_length) && (i < MAX_INSTRUCTION_LENGTH); i++) 
    {
        this->SyncPrintFormat(output_file, "%02x", instruction.bytes[i]);
        
        if (!prefix_flag && i < (MAX_INSTRUCTION_LENGTH - 1) &&
            InstructionManager::IsPrefix(instruction.bytes[i]) &&
            !InstructionManager::IsPrefix(instruction.bytes[i + 1]))
        {
            this->SyncPrintFormat(output_file, " ");
            prefix_flag = true;
        }
    }
}

void OutputManager::GiveResultToOutput(const Instruction& instruction, BuildMode build_mode, FILE* output_file)
{
    const uint8_t* code = instruction.bytes.data();
    size_t code_size = instruction.bytes.size();
    uint64_t address = (uintptr_t)this->_packet_buffer;
    
    switch (this->_output_mode) {
        case OutputMode::text:
            switch (build_mode) 
            {
                case BuildMode::BruteForce:
                case BuildMode::TunnelMinMax:
                case BuildMode::Random:
                    this->SyncPrintFormat(output_file, " %s", _expected_length == this->_current_result.length ? " " : ".");
                    this->SyncPrintFormat(output_file, "r: (%2d) ", this->_current_result.length);
                    if (this->_current_result.signum==SIGILL)  { this->SyncPrintFormat(output_file, "sigill "); }
                    if (this->_current_result.signum==SIGSEGV) { this->SyncPrintFormat(output_file, "sigsegv"); }
                    if (this->_current_result.signum==SIGFPE)  { this->SyncPrintFormat(output_file, "sigfpe "); }
                    if (this->_current_result.signum==SIGBUS)  { this->SyncPrintFormat(output_file, "sigbus "); }
                    if (this->_current_result.signum==SIGTRAP) { this->SyncPrintFormat(output_file, "sigtrap"); }
                    this->SyncPrintFormat(output_file, " %3d ", this->_current_result.si_code);
                    this->SyncPrintFormat(output_file, " %08x ", this->_current_result.address);
                    this->PrintInMcToOutput(instruction, this->_current_result.length, output_file);
                    this->SyncPrintFormat(output_file, "\n");
                    break;
                    
                default:
                    assert(0);
            }
            break;
        
        case OutputMode::raw:
#if USE_CAPSTONE
            if (cs_disasm_iter(this->_capstone_handle, (const uint8_t**)&code, &code_size, &address, this->_capstone_insn)) 
            {
#if RAW_REPORT_DISAS_MNE
                strncpy(this->_disassembly_info.mne, this->_capstone_insn[0].mnemonic, RAW_DISAS_MNEMONIC_BYTES);
#endif
#if RAW_REPORT_DISAS_OPS
                strncpy(this->_disassembly_info.ops, capstone_insn[0].op_str, RAW_DISAS_OP_BYTES);
#endif
#if RAW_REPORT_DISAS_LEN
                this->_disassembly_info.length = (int)(address - (uintptr_t)this->_packet_buffer);
#endif
#if RAW_REPORT_DISAS_VAL
                this->_disassembly_info.value = true;
#endif
            }
            else 
            {
#if RAW_REPORT_DISAS_MNE
                strncpy(this->_disassembly_info.mne, "(unk)", RAW_DISAS_MNEMONIC_BYTES);
#endif
#if RAW_REPORT_DISAS_OPS
                strncpy(this->_disassembly_info.ops, " ", RAW_DISAS_OP_BYTES);
#endif
#if RAW_REPORT_DISAS_LEN
                this->_disassembly_info.length = (int)(address - (uintptr_t)this->_packet_buffer);
#endif
#if RAW_REPORT_DISAS_VAL
                this->_disassembly_info.value = false;
#endif
            }
#if RAW_REPORT_DISAS_MNE || RAW_REPORT_DISAS_OPS || RAW_REPORT_DISAS_LEN
            this->SyncWriteBuffer(&this->_disassembly_info, sizeof(this->_disassembly_info), 1, stdout);
#endif
#endif
            this->SyncWriteBuffer(instruction.bytes.data(), RAW_REPORT_INSN_BYTES, 1, stdout);
            this->SyncWriteBuffer(&this->_current_result, sizeof(this->_current_result), 1, stdout);
            break;
            
        default:
            assert(0);
    }
    this->SyncFlushOutput(stdout, false);
}


void OutputManager::SyncPrintFormat(FILE* output_file, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (output_file == stdout) 
    {
        this->_stdout_buffer_pos += vsprintf(this->_stdout_buffer_pos, format, args);
    }
    else if (output_file == stderr) 
    {
        this->_stderr_buffer_pos += vsprintf(this->_stderr_buffer_pos, format, args);
    }
    else 
    {
        assert(0);
    }

    va_end(args);
}

void OutputManager::SyncWriteBuffer(const void* output_buffer, size_t single_element_size, size_t elements_count_to_write, FILE* output_file)
{
    size_t size = single_element_size * elements_count_to_write;
    
    if (output_file == stdout)
    {
        memcpy(this->_stdout_buffer_pos, output_buffer, size);
        this->_stdout_buffer_pos += size;
    }
    else if (output_file == stderr)
    {
        memcpy(this->_stderr_buffer_pos, output_buffer, size);
        this->_stderr_buffer_pos += size;
    }
    else 
    {
        assert(0);
    }
}

void OutputManager::SyncFlushOutput(FILE* output_file, bool force)
{
    if (output_file == stdout)
    {
        this->_stdout_sync_counter++;
        if (this->_stdout_sync_counter == SYNC_LINES_STDOUT || force) 
        {
            this->_stdout_sync_counter = 0;
            
            pthread_mutex_lock(this->_output_mutex);
            
            fwrite(this->_stdout_buffer, this->_stdout_buffer_pos-this->_stdout_buffer, 1, output_file);
            fflush(output_file);
            
            pthread_mutex_unlock(this->_output_mutex);
            
            this->_stdout_buffer_pos=this->_stdout_buffer;
        }
    }
    else if (output_file == stderr)
    {
        this->_stderr_sync_counter++;
        if (this->_stderr_sync_counter == SYNC_LINES_STDERR || force) 
        {
            this->_stderr_sync_counter=0;
            
            pthread_mutex_lock(this->_output_mutex);
            
            fwrite(this->_stderr_buffer, this->_stderr_buffer_pos-this->_stderr_buffer, 1, output_file);
            fflush(output_file);
            
            pthread_mutex_unlock(this->_output_mutex);
           
            this->_stderr_buffer_pos=this->_stderr_buffer;
        }
    }
    else
    {
        assert(0);
    }
}


void OutputManager::PrintInstructionToOutput(const Instruction& instruction, FILE* output_file)
{
    for (size_t i = 0; i < sizeof(instruction.bytes); i++) 
    {
        this->SyncPrintFormat(output_file, "%02x", instruction.bytes[i]);
    }
}

void OutputManager::PrintRangeToOutput(const InstructionRange &range, FILE* output_file)
{
    this->PrintInstructionInAsmToOutput(range.start, output_file);
    this->SyncPrintFormat(output_file, ";");
    this->PrintInstructionInAsmToOutput(range.end, output_file);
}

#if USE_CAPSTONE
int OutputManager::PrintInstructionInAsmToOutput(const Instruction& instruction, FILE* output_file)
{
    if (this->_capstone_insn == nullptr)
    {
        return 0;
    }
    
    if (this->_output_mode == OutputMode::text) 
    {
        const uint8_t* code = instruction.bytes.data();
        size_t code_size = instruction.bytes.size();
        uint64_t address = (uintptr_t)this->_packet_buffer;

        if (cs_disasm_iter(this->_capstone_handle, (const uint8_t**)&code, &code_size, &address, this->_capstone_insn)) 
        {
            this->SyncPrintFormat(output_file, "%10s %-45s (%2d)", this->_capstone_insn[0].mnemonic, this->_capstone_insn[0].op_str,
                (int)(address - (uintptr_t)this->_packet_buffer));
        }
        else
        {
            this->SyncPrintFormat(output_file, "%10s %-45s (%2d)", "(unk)", " ", 
                (int)(address - (uintptr_t)this->_packet_buffer));
        }
        _expected_length = (int)(address - (uintptr_t)this->_packet_buffer);
    }

    return 0;
}

#endif

void OutputManager::Tick(const Instruction& instruction, BuildMode build_mode)
{
    static uint64_t tick_counter = 0;
    
    if (ConfigManager::Instance().GetConfig().should_show_tick) 
    {
        tick_counter++;
        
        if ((tick_counter & uint64_t(TICK_MASK)) == 0)
        {
            this->SyncPrintFormat(stderr, "t: ");
            this->PrintInMcToOutput(instruction, 8, stderr);
            this->SyncPrintFormat(stderr, "... ");
#if USE_CAPSTONE
            this->PrintInstructionInAsmToOutput(instruction, stderr);
            this->SyncPrintFormat(stderr, "\t");
#endif
            this->GiveResultToOutput(instruction, build_mode, stderr);
            this->SyncFlushOutput(stderr, false);
        }
    }
}

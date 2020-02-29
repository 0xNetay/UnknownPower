//
// Created by student on 1/29/20.
//

#include "InstructionManager.hpp"
#include "ConfigManager/ConfigManager.hpp"
#include "OutputManager/OutputManager.hpp"

bool InstructionManager::Init()
{
    this->_current_instruction.bytes = { 0 };
    this->_current_instruction.length = 0;
    this->_current_index = -1;
    this->_last_length = -1;
    return true;
}

bool InstructionManager::Init(const Instruction &other)
{
    if (!this->Init())
    {
        return false;
    }

    this->_current_instruction = other;
    return true;
}

bool InstructionManager::BuildNextRange()
{
    switch (this->GetBuildMode())
    {
        case BuildMode::BruteForce:
            if (!this->_had_started) // First call - init
            {
                this->Init(this->_current_search_range.start);
                this->_current_index = ConfigManager::Instance().GetConfig().brute_force_byte_depth - 1;
            }
            else // Starting from deepest depth, finding the first byte that had not wrapped around
            {
                for (this->_current_index = ConfigManager::Instance().GetConfig().brute_force_byte_depth - 1;
                     this->_current_index >= 0;
                     this->_current_index--)
                {
                    this->_current_instruction.bytes[this->_current_index]++;

                    if (this->_current_instruction.bytes[this->_current_index])
                    {
                        break;
                    }
                }
            }
            break;

        case BuildMode::Random:
            if (!this->_had_started) // First call - init
            {
                this->Init();
            }

            // Any call - new random (first random for first call)
            this->BuildRandomInstruction();
            break;

        case BuildMode::TunnelMinMax:
            if (!this->_had_started) // First call - init
            {
                this->Init(this->_current_search_range.start);
                this->_current_index = this->_current_search_range.start.length;
            }
            else // For any new call, if the last instruction found a new length, we go deeper
                // otherwise, we found the current deepest length of the current instruction
            {
                if (OutputManager::Instance().GetImmutableResult().length != this->_last_length &&
                    this->_current_index < OutputManager::Instance().GetImmutableResult().length - 1)
                {
                    this->_current_index++;
                }

                this->_last_length = OutputManager::Instance().GetImmutableResult().length;

                this->_current_instruction.bytes[this->_current_index]++;

                while (this->_current_index >= 0 &&
                       this->_current_instruction.bytes[this->_current_index] == 0)
                {
                    this->_current_index--;
                    if (this->_current_index >= 0)
                    {
                        this->_current_instruction.bytes[this->_current_index]++;
                    }

                    // We found a new length level, so we reset the last length
                    this->_last_length = -1;
                }
            }
            break;

        default:
            assert(0);
    }

    // From now on, calling this is a new iteration and not init
    this->_had_started = true;

    int i = 0;
    while (ConfigManager::Instance().GetAtOpcodeBlacklist(i).opcode)
    {
        if (this->HasOpcode((uint8_t*)ConfigManager::Instance().GetAtOpcodeBlacklist(i).opcode))
        {
            switch (OutputManager::Instance().GetOutputMode())
            {
                case OutputMode::Text:
                    OutputManager::Instance().SyncPrintFormat(stdout, "x: ");
                    OutputManager::Instance().PrintInMcToOutput(this->_current_instruction, MAX_INSTRUCTION_LENGTH, stdout);
                    OutputManager::Instance().SyncPrintFormat(stdout, "... (%s)\n", ConfigManager::Instance().GetAtOpcodeBlacklist(i).reason);
                    OutputManager::Instance().SyncFlushOutput(stdout, false);
                    break;

                case OutputMode::Raw:
                    OutputManager::Instance().GetMutableResult() = {0, 0, 0, 0, 0};
                    OutputManager::Instance().GiveResultToOutput(this->_current_instruction, this->_build_mode, stdout);
                    break;

                default:
                    assert(0);
            }
            return this->BuildNextInstruction();
        }
        i++;
    }

    i = 0;
    while (ConfigManager::Instance().GetAtPrefixBlacklist(i).prefix)
    {
        if (this->HasPrefix((uint8_t*)ConfigManager::Instance().GetAtPrefixBlacklist(i).prefix))
        {
            switch (OutputManager::Instance().GetOutputMode())
            {
                case OutputMode::Text:
                    OutputManager::Instance().SyncPrintFormat(stdout, "x: ");
                    OutputManager::Instance().PrintInMcToOutput(this->_current_instruction, MAX_INSTRUCTION_LENGTH, stdout);
                    OutputManager::Instance().SyncPrintFormat(stdout, "... (%s)\n", ConfigManager::Instance().GetAtPrefixBlacklist(i).reason);
                    OutputManager::Instance().SyncFlushOutput(stdout, false);
                    break;

                case OutputMode::Raw:
                    OutputManager::Instance().GetMutableResult() = {0, 0, 0, 0, 0};
                    OutputManager::Instance().GiveResultToOutput(this->_current_instruction, this->_build_mode, stdout);
                    break;
                default:
                    assert(0);
            }
            return this->BuildNextInstruction();
        }
        i++;
    }

    if (this->PrefixCount() > ConfigManager::Instance().GetConfig().max_explored_prefix ||
        (!ConfigManager::Instance().GetConfig().allow_exploring_more_than_one_prefix_in_search && this->HasDuplicatePrefix()))
    {
        switch (OutputManager::Instance().GetOutputMode())
        {
            case OutputMode::Text:
                OutputManager::Instance().SyncPrintFormat(stdout, "x: ");
                OutputManager::Instance().PrintInMcToOutput(this->_current_instruction, MAX_INSTRUCTION_LENGTH, stdout);
                OutputManager::Instance().SyncPrintFormat(stdout, "... (%s)\n", "prefix violation");
                OutputManager::Instance().SyncFlushOutput(stdout, false);
                break;

            case OutputMode::Raw:
                OutputManager::Instance().GetMutableResult() = {0, 0, 0, 0, 0};
                OutputManager::Instance().GiveResultToOutput(this->_current_instruction, this->_build_mode, stdout);
                break;

            default:
                assert(0);
        }
        return this->BuildNextInstruction();
    }

    if (memcmp(this->_current_instruction.bytes.data(),
            this->_current_search_range.end.bytes.data(),
            sizeof(this->_current_instruction.bytes)) >= 0)
    {
        return false;
    }

    switch (this->GetBuildMode())
    {
        case BuildMode::Random:
            return true;
        case BuildMode::BruteForce:
        case BuildMode::TunnelMinMax:
            return this->_current_index >= 0;
        default:
            assert(0);
    }
}

bool InstructionManager::BuildNextInstruction()
{
    bool result = true;

    switch (this->GetBuildMode())
    {
        case BuildMode::Random:
            if (this->_had_started)
            {
                result = false;
            }
            else
            {
                this->_current_search_range = TOTAL_RANGE;
            }
            break;

        case BuildMode::BruteForce:
        case BuildMode::TunnelMinMax:
            pthread_mutex_lock(this->_pool_mutex);
            this->_had_started = false;

            if (memcmp(this->_range_marker->bytes.data(), TOTAL_RANGE.end.bytes.data(),
                       sizeof(this->_range_marker->bytes))==0)
            {
                // reached end of range
                result = false;
            }
            else
            {
                this->_current_search_range.start = *this->_range_marker;
                this->_current_search_range.end = *this->_range_marker;

                if (!IncrementRangeForNext(this->_current_search_range.end, ConfigManager::Instance().GetConfig().range_bytes))
                {
                    // if increment rolled over, set to end
                    this->_current_search_range.end = TOTAL_RANGE.end;
                }
                else if (memcmp(this->_current_search_range.end.bytes.data(),
                                TOTAL_RANGE.end.bytes.data(), sizeof(this->_current_search_range.end.bytes))>0)
                {
                    // if increment moved past end, set to end
                    this->_current_search_range.end = TOTAL_RANGE.end;
                }

                *this->_range_marker= this->_current_search_range.end;
            }

            pthread_mutex_unlock(this->_pool_mutex);
            break;

        default:
            assert(0);
    }

    return result;
}

bool InstructionManager::BuildRandomInstruction()
{
    static uint8_t inclusive_end[MAX_INSTRUCTION_LENGTH];

    memcpy(inclusive_end, this->_current_search_range.end.bytes.data(), MAX_INSTRUCTION_LENGTH);

    int i = MAX_INSTRUCTION_LENGTH - 1;
    while (i >= 0)
    {
        inclusive_end[i]--;
        if (inclusive_end[i] != 0xff)
        {
            break;
        }
        i--;
    }

    bool all_max = true;
    bool all_min = true;
    for (i = 0; i < MAX_INSTRUCTION_LENGTH; i++)
    {
        int random = rand();
        if (all_max && all_min)
        {
            this->_current_instruction.bytes[i] =
                    random %
                    (inclusive_end[i] - this->_current_search_range.start.bytes[i] + 1) +
                    this->_current_search_range.start.bytes[i];
        }
        else if (all_max)
        {
            this->_current_instruction.bytes[i] = random % (inclusive_end[i] + 1);
        }
        else if (all_min)
        {
            this->_current_instruction.bytes[i] =
                    random %
                    (256 - this->_current_search_range.start.bytes[i]) +
                    this->_current_search_range.start.bytes[i];
        }
        else
        {
            this->_current_instruction.bytes[i] = random % 256;
        }
        all_max=all_max&&(this->_current_instruction.bytes[i] == inclusive_end[i]);
        all_min=all_min&&(this->_current_instruction.bytes[i] == this->_current_search_range.start.bytes[i]);
    }

    return true;
}

bool InstructionManager::IncrementRangeForNext(Instruction &instruction, int marker)
{
    // Zeroing the instruction bytes from marker to end
    for (size_t i = marker; i < sizeof(Instruction::bytes); i++)
    {
        instruction.bytes[i] = 0;
    }

    int i = marker - 1;

    if (i >= 0)
    {
        instruction.bytes[i]++;
        while (instruction.bytes[i] == 0)
        {
            i--;
            if (i < 0)
            {
                break;
            }
            instruction.bytes[i]++;
        }
    }

    instruction.length = marker;

    return i >= 0;
}

bool InstructionManager::CreateRanges()
{
    if (_range_marker == nullptr)
    {
        this->_range_marker = reinterpret_cast<Instruction*>(mmap(NULL,sizeof(*this->_range_marker),
                PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0));
        *this->_range_marker = TOTAL_RANGE.start;
    }

    return true;
}

bool InstructionManager::DropRanges()
{
    if (this->_range_marker != nullptr)
    {
        munmap(this->_range_marker, sizeof *this->_range_marker);
    }

    return true;
}

bool InstructionManager::IsPrefix(uint8_t prefix)
{
    return
            prefix==0xf0 || /* lock */
            prefix==0xf2 || /* repne / bound */
            prefix==0xf3 || /* rep */
            prefix==0x2e || /* cs / branch taken */
            prefix==0x36 || /* ss / branch not taken */
            prefix==0x3e || /* ds */
            prefix==0x26 || /* es */
            prefix==0x64 || /* fs */
            prefix==0x65 || /* gs */
            prefix==0x66 || /* data */
            prefix==0x67    /* addr */
#if __x86_64__
            || (prefix >= 0x40 && prefix <= 0x4f) /* rex */
#endif
            ;
            // TODO: POWERPC
}

size_t InstructionManager::PrefixCount()
{
    for (size_t i = 0; i < MAX_INSTRUCTION_LENGTH; i++)
    {
        if (!InstructionManager::IsPrefix(this->_current_instruction.bytes[i]))
        {
            return i;
        }
    }
    return 0;
}

bool InstructionManager::HasDuplicatePrefix()
{
    static constexpr size_t MAX_BYTES_COUNT = 0xff;
    static size_t byte_count[MAX_BYTES_COUNT];

    memset(byte_count, 0, sizeof(byte_count));

    for (size_t i = 0; i < MAX_INSTRUCTION_LENGTH; i++)
    {
        if (InstructionManager::IsPrefix(this->_current_instruction.bytes[i]))
        {
            byte_count[this->_current_instruction.bytes[i]]++;
        }
        else
        {
            break;
        }
    }

    for (size_t byte : byte_count)
    {
        if (byte > 1)
        {
            return true;
        }
    }

    return false;
}

bool InstructionManager::HasOpcode(const uint8_t original_opcode[])
{
    if (original_opcode == nullptr)
    {
        return false;
    }

    for (size_t i = 0; i < MAX_INSTRUCTION_LENGTH; i++)
    {
        if (!InstructionManager::IsPrefix(this->_current_instruction.bytes[i]))
        {
            size_t j = 0;
            do
            {
                if ((i + j) >= MAX_INSTRUCTION_LENGTH || original_opcode[j] != this->_current_instruction.bytes[i + j])
                {
                    return false;
                }

                j++;
            } while (original_opcode[j]);

            return true;
        }
    }
    return false;
}

bool InstructionManager::HasPrefix(const uint8_t original_prefix[])
{
    if (original_prefix == nullptr)
    {
        return false;
    }

    for (size_t i = 0; i < MAX_INSTRUCTION_LENGTH; i++)
    {
        if (InstructionManager::IsPrefix(this->_current_instruction.bytes[i]))
        {
            size_t j = 0;
            do
            {
                if (this->_current_instruction.bytes[i] == original_prefix[j])
                {
                    return true;
                }
                j++;
            } while (original_prefix[j]);
        }
        else
        {
            return false;
        }
    }
    return false;
}

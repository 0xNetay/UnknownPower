//
// Created by student on 1/29/20.
//

#include "Builder.hpp"

bool Builder::Init()
{
    this->_current_instruction.bytes = { 0 };
    this->_current_instruction.length = 0;
    this->_current_index = -1;
    this->_last_length = -1;
    return true;
}

bool Builder::Init(const Instruction &other)
{
    if (!this->Init())
    {
        return false;
    }

    this->_current_instruction = other;
    return true;
}

bool Builder::BuildNextRange()
{
    switch (this->GetBuildMode())
    {
        case BuildMode::BruteForce:
            if (!this->_had_started) // First call - init
            {
                this->Init(this->_current_search_range.start);
                this->_current_index = this->_config.brute_force_byte_depth - 1;
            }
            else // Starting from deepest depth, finding the first byte that had not wrapped around
            {
                for (this->_current_index = this->_config.brute_force_byte_depth - 1;
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
                if (this->_result.length != this->_last_length &&
                    this->_current_index < this->_result.length - 1)
                {
                    this->_current_index++;
                }

                this->_last_length = this->_result.length;

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
    while (opcode_blacklist[i].opcode) {
        if (has_opcode((uint8_t*)opcode_blacklist[i].opcode)) {
            switch (output) {
                case TEXT:
                    sync_fprintf(stdout, "x: "); print_mc(stdout, 16);
                    sync_fprintf(stdout, "... (%s)\n", opcode_blacklist[i].reason);
                    sync_fflush(stdout, false);
                    break;
                case RAW:
                    result=(result_t){0,0,0,0,0};
                    give_result(stdout);
                    break;
                default:
                    assert(0);
            }
            return this->BuildNextInstruction();
        }
        i++;
    }

    i = 0;
    while (prefix_blacklist[i].prefix) {
        if (has_prefix((uint8_t*)prefix_blacklist[i].prefix)) {
            switch (output) {
                case TEXT:
                    sync_fprintf(stdout, "x: "); print_mc(stdout, 16);
                    sync_fprintf(stdout, "... (%s)\n", prefix_blacklist[i].reason);
                    sync_fflush(stdout, false);
                    break;
                case RAW:
                    result=(result_t){0,0,0,0,0};
                    give_result(stdout);
                    break;
                default:
                    assert(0);
            }
            return this->BuildNextInstruction();
        }
        i++;
    }

    if (prefix_count() > this->_config.max_prefix ||
        (!this->_config.allow_dup_prefix && has_dup_prefix())) {
        switch (output) {
            case TEXT:
                sync_fprintf(stdout, "x: "); print_mc(stdout, 16);
                sync_fprintf(stdout, "... (%s)\n", "prefix violation");
                sync_fflush(stdout, false);
                break;
            case RAW:
                result=(result_t){0,0,0,0,0};
                give_result(stdout);
                break;
            default:
                assert(0);
        }
        return this->BuildNextInstruction();
    }

    /* early exit */
    /* check if we are at, or past, the end instruction */
    if (memcmp(this->_current_instruction.bytes.data(),
            this->_current_search_range.end.bytes.data(),
            sizeof(this->_current_instruction.bytes)) >= 0)
    {
        return false;
    }

    /* search based exit */
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

bool Builder::BuildNextInstruction()
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

                if (!IncrementRangeForNext(this->_current_search_range.end, this->_config.range_bytes))
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

bool Builder::BuildRandomInstruction()
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

bool Builder::IncrementRangeForNext(Instruction &instruction, int marker)
{
    // Zeroing the instruction bytes from marker to end
    for (int i = marker; i < sizeof(Instruction::bytes); i++)
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

bool Builder::CreateRanges()
{
    if (_range_marker == nullptr)
    {
        this->_range_marker = reinterpret_cast<Instruction*>(mmap(NULL,sizeof(*this->_range_marker),
                PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0));
        *this->_range_marker = TOTAL_RANGE.start;
    }
}

bool Builder::DropRanges()
{
    if (this->_range_marker != nullptr)
    {
        munmap(this->_range_marker, sizeof *this->_range_marker);
    }
}


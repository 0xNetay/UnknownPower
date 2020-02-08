//
// Created by student on 1/29/20.
//

#ifndef UNKNOWNPOWER_BUILDER_HPP
#define UNKNOWNPOWER_BUILDER_HPP

#include "Definitions.hpp"
#include "config/Definitions.hpp"

class Builder
{
public:
    inline explicit Builder(pthread_mutex_t* pool_mutex) : _pool_mutex(pool_mutex) {}
    inline ~Builder() { this->DropRanges(); }

    bool CreateRanges();
    bool DropRanges();

    bool BuildNextRange();
    bool BuildNextInstruction();

    inline BuildMode GetBuildMode() { return this->_build_mode; }
    inline void SetBuildMode(BuildMode new_mode) { this->_build_mode = new_mode; }

private:
    bool Init();
    bool Init(const Instruction &other);

    bool BuildRandomInstruction();

    bool IncrementRangeForNext(Instruction &instruction, int marker);

    bool IsPrefix(uint8_t prefix);
    size_t PrefixCount();
    bool HasDuplicatePrefix();
    bool HasOpcode(const uint8_t original_opcode[]);
    bool HasPrefix(const uint8_t original_prefix[]);

    pthread_mutex_t* _pool_mutex;

    Instruction _current_instruction = {};
    int _current_index = 0;
    int _last_length = 0;

    Instruction* _range_marker = nullptr;
    InstructionRange _current_search_range = {};

    bool _had_started = false;
    BuildMode _build_mode = BuildMode::TunnelMinMax;
};

#endif //UNKNOWNPOWER_BUILDER_HPP

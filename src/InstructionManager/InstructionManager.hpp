//
// Created by student on 1/29/20.
//

#ifndef UNKNOWNPOWER_INSTRUCTIONMANAGER_HPP
#define UNKNOWNPOWER_INSTRUCTIONMANAGER_HPP

#include "Definitions.hpp"
#include "ConfigManager/Definitions.hpp"

class InstructionManager
{
public:
    inline explicit InstructionManager(pthread_mutex_t* pool_mutex) : _pool_mutex(pool_mutex) {}
    inline ~InstructionManager() { this->DropRanges(); }

    bool CreateRanges();
    bool DropRanges();

    bool BuildNextRange();
    bool BuildNextInstruction();

    inline const Instruction& GetCurrentInstruction() { return this->_current_instruction; }
    inline const InstructionRange& GetCurrentRange() { return this-> _current_search_range; }

    inline BuildMode GetBuildMode() { return this->_build_mode; }
    inline void SetBuildMode(BuildMode new_mode) { this->_build_mode = new_mode; }

    static bool IsPrefix(uint8_t prefix);

private:
    bool Init();
    bool Init(const Instruction &other);

    bool BuildRandomInstruction();

    bool IncrementRangeForNext(Instruction &instruction, int marker);

    size_t PrefixCount();
    bool HasDuplicatePrefix();
    bool HasOpcode(const uint8_t original_opcode[]);
    bool HasPrefix(const uint8_t original_prefix[]);

    pthread_mutex_t* _pool_mutex;

    Instruction _current_instruction = {};
    size_t _current_index = 0;
    size_t _last_length = 0;

    Instruction* _range_marker = nullptr;
    InstructionRange _current_search_range = {};

    bool _had_started = false;
    BuildMode _build_mode = BuildMode::TunnelMinMax;
};

#endif //UNKNOWNPOWER_INSTRUCTIONMANAGER_HPP

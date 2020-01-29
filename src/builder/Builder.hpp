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
    inline Builder(Config &config, Result &result, pthread_mutex_t* pool_mutex) :
        _config(config), _result(result), _pool_mutex(pool_mutex)
    {

    }
    inline ~Builder()
    {
        this->DropRanges();
    }

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

    Config &_config;
    Result &_result;
    pthread_mutex_t* _pool_mutex;

    Instruction _current_instruction;
    int _current_index = 0;
    int _last_length = 0;

    Instruction* _range_marker = nullptr;
    InstructionRange _current_search_range;

    bool _had_started = false;
    BuildMode _build_mode = BuildMode::TunnelMinMax;
};

#endif //UNKNOWNPOWER_BUILDER_HPP

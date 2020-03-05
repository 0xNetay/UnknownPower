//
// Created by student on 1/29/20.
//

#ifndef UNKNOWNPOWER_INSTRUCTIONMANAGER_DEFINITIONS_HPP
#define UNKNOWNPOWER_INSTRUCTIONMANAGER_DEFINITIONS_HPP

#include <array>
#include "General.hpp"

enum class BuildMode
{
    BruteForce,
    Random,
    TunnelMinMax,
};

using InstructionBytes = std::array<uint8_t, MAX_INSTRUCTION_LENGTH>;

struct Instruction
{
    InstructionBytes bytes;
    size_t length;
};

struct InstructionRange
{
    Instruction start;
    Instruction end;
};

namespace details
{
#if PROCESSOR == POWER_PC
    const Instruction START_INSTRUCTION =
    {
        .bytes = { 0x00, 0x00, 0x00, 0x00 },
        .length = 0
    };
    const Instruction END_INSTRUCTION =
    {
        .bytes = { 0xff, 0xff, 0xff, 0xff },
        .length = 0
    };
#elif PROCESSOR == INTEL
    const Instruction START_INSTRUCTION =
    {
        .bytes = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        .length = 0
    };
    const Instruction END_INSTRUCTION =
    {
        .bytes = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        .length = 0
    };
#endif
}

const InstructionRange TOTAL_RANGE = { details::START_INSTRUCTION, details::END_INSTRUCTION };

#endif //UNKNOWNPOWER_INSTRUCTIONMANAGER_DEFINITIONS_HPP

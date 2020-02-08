//
// Created by student on 1/29/20.
//

#ifndef UNKNOWNPOWER_BUILDER_DEFINITIONS_HPP
#define UNKNOWNPOWER_BUILDER_DEFINITIONS_HPP

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
    int length;
};

struct InstructionRange
{
    Instruction start;
    Instruction end;
};

namespace details
{
#if defined(POWER_PC)
    const Instruction START_INSTRUCTION = { { 0x00, 0x00, 0x00, 0x00 }, 0};
    const Instruction END_INSTRUCTION = { { 0xff, 0xff, 0xff, 0xff }, 0};
#elif defined(INTEL)
    const Instruction START_INSTRUCTION = {
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0};
    const Instruction END_INSTRUCTION = {
            {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0};
#endif
}

const InstructionRange TOTAL_RANGE = { details::START_INSTRUCTION, details::END_INSTRUCTION };

#endif //UNKNOWNPOWER_BUILDER_DEFINITIONS_HPP

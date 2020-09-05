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
    /* Creates a new instruction builder instance */
    inline explicit InstructionManager(pthread_mutex_t** pool_mutex) : _pool_mutex(pool_mutex) {}

    /* If goes out of scope drop ranges just in case it wasn't called beforehand */
    inline ~InstructionManager() { this->DropRanges(); }

    /* Allocate memory for the possible ranges */
    bool CreateRanges();

    /* Free the ranges memory */
    bool DropRanges();

    /* Access the Total Range */
    inline const InstructionRange& GetTotalRange() { return this-> _total_range; }
    inline void SetTotalRange(const InstructionRange& other) { this-> _total_range = other; }

    /* Create the next range of instructions according to the build mode */
    bool BuildNextRange();

    /* Generates the next instruction opcode according to the build mode and current range of instructions */
    bool BuildNextInstruction();

    /* Access to instruction and range */
    inline const Instruction& GetCurrentInstruction() { return this->_current_instruction; }
    inline const InstructionRange& GetCurrentRange() { return this-> _current_search_range; }

    /* Access to the instruction build mode */
    inline BuildMode GetBuildMode() { return this->_build_mode; }
    inline void SetBuildMode(BuildMode new_mode) { this->_build_mode = new_mode; }

    /* Helper function to check if a given byte can be prefix of an instruction */
    static bool IsPrefix(uint8_t prefix);

private:
    /* Initiate the instruction manager (setting up the current instruction to starting position */
    bool Init();
    bool Init(const Instruction &other);

    /* Generates a random instruction for the random build mode */
    bool BuildRandomInstruction();

    /* Moves the instruction into the next one in the range  */
    bool IncrementRangeForNext(Instruction &instruction, int marker);

    /* Helper functions over the current instruction to check for prefixes and blacklisted opcodes */
    size_t PrefixCount();
    bool HasDuplicatePrefix();
    bool HasOpcode(const uint8_t original_opcode[]);
    bool HasPrefix(const uint8_t original_prefix[]);

    /* Outsource */
    pthread_mutex_t** _pool_mutex;                  // Given mutex for ranges memory pool

    /* Instruction info */
    Instruction _current_instruction = {};          // Current instruction byte code and length
    int _current_index = 0;                         // Index of the last valid byte within the instruction buffer
    int _last_length = 0;                           // Length of the previous instruction

    /* Range info */
    InstructionRange _total_range = { details::START_INSTRUCTION, details::END_INSTRUCTION };
    Instruction* _range_marker = nullptr;           // Marker within a range to work with current instruction
    InstructionRange _current_search_range = {};    // Current range we build with

    /* Misc */
    bool _had_started = false;                      // If it is first time and we need to set things according to each mode
    BuildMode _build_mode = BuildMode::TunnelMinMax;
};

#endif //UNKNOWNPOWER_INSTRUCTIONMANAGER_HPP

//
// Created by student on 1/29/20.
//

#ifndef UNKNOWNPOWER_CONFIGMANAGER_HPP
#define UNKNOWNPOWER_CONFIGMANAGER_HPP

#include "Definitions.hpp"

class ConfigManager
{
public:
    /* One instance for all across the system */
    inline static ConfigManager& Instance()
    {
        static ConfigManager configManager;
        return configManager;
    }

    /* Access to the system configuration */
    inline void SetConfig(const Config& config) { _config = config; }
    inline const Config& GetConfig() { return _config; }

    /* Access to the default states of the registers */
    inline void SetRegState(const RegState& reg_state) { _default_reg_state = reg_state; }
    inline const RegState& GetRegState() { return _default_reg_state; }

    /* Adds a new opcode to ignore (skipped during instruction building) */
    inline bool AppendOpcodeBlackList(const IgnoredOpcode& ignored)
    {
        if (_opcode_blacklist_length >= MAX_BLACKLISTED_OPCODES)
        {
            return false;
        }

        _opcode_blacklist[_opcode_blacklist_length] = ignored;
        _opcode_blacklist_length++;
        return true;
    }

    /* Sets initial values to the ignored opcode list (skipped during instruction building) */
    inline void SetOpcodeBlackList(const IgnoredOpcode (&opcode_blacklist)[MAX_BLACKLISTED_OPCODES], size_t opcode_blacklist_length)
    {
        memcpy(_opcode_blacklist, opcode_blacklist, MAX_BLACKLISTED_OPCODES);
        _opcode_blacklist_length = opcode_blacklist_length;
    }

    /* Iterate over the opcode black list with a get at method */
    inline const IgnoredOpcode& GetAtOpcodeBlacklist(size_t index)
    {
        if (index >= _opcode_blacklist_length)
        {
            return { ConfigManager::EMPTY_IGNORED_OPCODE };
        }

        return _opcode_blacklist[index];
    }

    /* Returns how many opcodes are in the ignore list */
    inline size_t GetOpcodeBlacklistLength() { return _opcode_blacklist_length; }

    /* Adds a new prefix to ignore (skipped during instruction building) */
    inline bool AppendPrefixBlackList(const IgnoredPrefix& ignored)
    {
        if (_prefix_blacklist_length >= MAX_BLACKLISTED_PREFIXES)
        {
            return false;
        }

        _prefix_blacklist[_prefix_blacklist_length] = ignored;
        _prefix_blacklist_length++;
        return true;
    }

    /* Sets initial values to the ignored prefix list (skipped during instruction building) */
    inline void SetPrefixBlackList(const IgnoredPrefix (&prefix_blacklist)[MAX_BLACKLISTED_PREFIXES], size_t prefix_blacklist_length)
    {
        memcpy(_prefix_blacklist, prefix_blacklist, MAX_BLACKLISTED_PREFIXES);
        _prefix_blacklist_length = prefix_blacklist_length;
    }

    /* Iterate over the prefix black list with a get at method */
    inline const IgnoredPrefix& GetAtPrefixBlacklist(size_t index)
    {
        if (index >= _prefix_blacklist_length)
        {
            return { ConfigManager::EMPTY_IGNORED_PREFIX };
        }

        return _prefix_blacklist[index];
    }

    /* Returns how many prefixes are in the ignore list */
    inline size_t GetPrefixBlacklistLength() { return _prefix_blacklist_length; }

private:
    /* Singleton class, no instances are allowed */
    inline ConfigManager() = default;

    /* In & Outsource */
    Config _config = {};                // System configuration
    RegState _default_reg_state = {};   // Registers state before injections

    /* Opcode Blacklist Outsource */
    IgnoredOpcode _opcode_blacklist[MAX_BLACKLISTED_OPCODES] = {};
    size_t _opcode_blacklist_length = 0;
    const IgnoredOpcode EMPTY_IGNORED_OPCODE = {nullptr, nullptr};

    /* Prefix Blacklist Outsource */
    IgnoredPrefix _prefix_blacklist[MAX_BLACKLISTED_PREFIXES] = {};
    size_t _prefix_blacklist_length = 0;
    const IgnoredPrefix EMPTY_IGNORED_PREFIX = {nullptr, nullptr};
};

#endif //UNKNOWNPOWER_CONFIGMANAGER_HPP

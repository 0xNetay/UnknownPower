//
// Created by student on 2/8/20.
//

#ifndef UNKNOWNPOWER_OUTPUT_DEFINITIONS_HPP
#define UNKNOWNPOWER_OUTPUT_DEFINITIONS_HPP

#include "General.hpp"

// ----------------
// Output
// ----------------
struct __attribute__ ((packed)) Result
{
    uint32_t valid;
    uint32_t length;
    uint32_t signum;
    uint32_t si_code;
    uint32_t address;
};

enum class OutputMode
{
    Text,
    Raw,
};

struct __attribute__ ((packed)) DisassemblyInfo
{
#if RAW_REPORT_DISAS_MNE
    char mne[RAW_REPORT_DISAS_MNE_BYTES];
#endif
#if RAW_REPORT_DISAS_OPS
    char ops[RAW_REPORT_DISAS_OPS_BYTES];
#endif
#if RAW_REPORT_DISAS_LEN
    int length;
#endif
#if RAW_REPORT_DISAS_VAL
    int value;
#endif
};

#endif //UNKNOWNPOWER_OUTPUT_DEFINITIONS_HPP

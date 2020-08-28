//
// Created by student on 2/8/20.
//

#ifndef UNKNOWNPOWER_PROCESSOR_DEFINITIONS_HPP
#define UNKNOWNPOWER_PROCESSOR_DEFINITIONS_HPP

#include "General.hpp"

struct  __attribute__ ((aligned(PAGE_SIZE))) DummyStack
{
    uint64_t dummy_stack_hi[256];
    uint64_t dummy_stack_lo[256];
};

#endif //UNKNOWNPOWER_PROCESSOR_DEFINITIONS_HPP

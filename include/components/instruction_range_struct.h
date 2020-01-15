#pragma once

#include "instruction_struct.h"

typedef struct _range { 
    instruction_t start; 
    instruction_t end; 
    bool had_started; 
} instruction_range_t;

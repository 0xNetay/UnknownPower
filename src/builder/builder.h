#pragma once

#include "general.h"

void init_inj(const insn_t* new_insn);

bool move_next_instruction(void);
bool move_next_range(void);
bool increment_range(insn_t* insn, int marker);

void free_ranges(void);
void initialize_ranges(void);

void zero_insn_end(insn_t* insn, int marker);
void get_rand_insn_in_range(range_t* r);
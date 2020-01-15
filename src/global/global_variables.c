#include "global.h"

instruction_info_t current_instruction = {{{0}, 0}, 0, 0};

void* packet_buffer = NULL;

char* packet = NULL;

instruction_t *range_marker = NULL;

instruction_range_t search_range = {
	.start={{0}, 0},
	.end={{0}, 0}, 
	.had_started=false
};

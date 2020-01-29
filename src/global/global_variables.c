#include "global.h"

instruction_info_t current_instruction = {{{0}, 0}, 0, 0};

void* packet_buffer = NULL;
char* packet = NULL;

instruction_t *current_range_length_marker = NULL;
instruction_range_t current_search_range = {};

instruction_range_t total_searched_range = {
	{{0}, 0},
	{{0}, 0},
	false
};

int current_expected_length = 0;
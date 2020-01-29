#pragma once

#include "global.h"

#define LINE_BUFFER_SIZE 256
#define BUFFER_LINES 16
#define SYNC_LINES_STDOUT   BUFFER_LINES
#define SYNC_LINES_STDERR   BUFFER_LINES
extern char stdout_buffer[LINE_BUFFER_SIZE*BUFFER_LINES];
extern char* stdout_buffer_pos;
extern int stdout_sync_counter;
extern char stderr_buffer[LINE_BUFFER_SIZE*BUFFER_LINES];
extern char* stderr_buffer_pos;
extern int stderr_sync_counter;

#if USE_CAPSTONE
int print_asm(FILE* f);
#endif
void print_mc(FILE*, int);

void give_result(FILE*);

void sync_fprintf(FILE* f, const char *format, ...);
void sync_fwrite(const void* ptr, size_t size, size_t count, FILE* f);
void sync_fflush(FILE* f, bool force);
void print_insn(FILE* f, instruction_t* insn);
void print_range(FILE* f, instruction_range_t* range);

void tick(void);
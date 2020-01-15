#pragma once

typedef struct _ignored_opcode {
	char* opcode;
	char* reason;
} ignore_opcode_t;

typedef struct _ignored_prefix {
	char* prefix;
	char* reason;
} ignore_prefix_t;
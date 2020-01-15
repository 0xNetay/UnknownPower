#pragma once

typedef struct _config {
	/* main limit on search is # of prefixes to explore */
	bool allow_exploring_more_than_one_prefix_in_search; 
	int max_explored_prefix;
	int brute_force_byte_depth;
	long seed_for_random;
	int range_bytes;
	bool should_show_tick;
	int jobs_count;
	bool force_core;
	int core_count;
	bool enable_null_access;
	bool no_execute_support;
} config_t;
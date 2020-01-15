#include "builder.h"

void init_inj(const insn_t* new_insn)
{
	inj.i=*new_insn;
	inj.index=-1;
	inj.last_len=-1;
}

bool move_next_instruction(void)
{
	int i;

	switch (mode) {
		case RAND:
			if (!search_range.started) {
				init_inj(&null_insn);
				get_rand_insn_in_range(&search_range);
			}
			else {
				get_rand_insn_in_range(&search_range);
			}
			break;
		case BRUTE:
			if (!search_range.started) {
				init_inj(&search_range.start);
				inj.index=config.brute_depth-1;
			}
			else {
				for (inj.index=config.brute_depth-1; inj.index>=0; inj.index--) {
					inj.i.bytes[inj.index]++;
					if (inj.i.bytes[inj.index]) {
						break;
					}
				}
			}
			break;
		case TUNNEL:
			if (!search_range.started) {
				init_inj(&search_range.start);
				inj.index=search_range.start.len;
			}
			else {
				/* not a perfect algorithm; should really look at length
				 * patterns of oher bytes at current index, not "last" length;
				 * also situations in which this may not dig deep enough, should
				 * really be looking at no length changes for n bytes, not just
				 * last byte.  but it's good enough for now. */

				/* if the last iteration changed the instruction length, go deeper */
				/* but not if we're already as deep as the instruction goes */
				//TODO: should also count a change in the signal as a reason to
				//go deeper
				if (result.length!=inj.last_len && inj.index<result.length-1) {
					inj.index++;
				}
				inj.last_len=result.length;

				inj.i.bytes[inj.index]++;

				while (inj.index>=0 && inj.i.bytes[inj.index]==0) {
					inj.index--;
					if (inj.index>=0) {
						inj.i.bytes[inj.index]++;
					}
					/* new tunnel level, reset length */
					inj.last_len=-1;
				}
			}
			break;
		case DRIVEN:
			i=MAX_INSN_LENGTH;
			do {
				i-=fread(inj.i.bytes, 1, i, stdin);
			} while (i>0);
			break;
		default:
			assert(0);
	}
	search_range.started=true;

	i=0;
	while (opcode_blacklist[i].opcode) {
		if (has_opcode((uint8_t*)opcode_blacklist[i].opcode)) {
			switch (output) {
				case TEXT:
					sync_fprintf(stdout, "x: "); print_mc(stdout, 16);
					sync_fprintf(stdout, "... (%s)\n", opcode_blacklist[i].reason);
					sync_fflush(stdout, false);
					break;
				case RAW:
					result=(result_t){0,0,0,0,0};
					give_result(stdout);
					break;
				default:
					assert(0);
			}
			return move_next_instruction();
		}
		i++;
	}

	i=0;
	while (prefix_blacklist[i].prefix) {
		if (has_prefix((uint8_t*)prefix_blacklist[i].prefix)) {
			switch (output) {
				case TEXT:
					sync_fprintf(stdout, "x: "); print_mc(stdout, 16);
					sync_fprintf(stdout, "... (%s)\n", prefix_blacklist[i].reason);
					sync_fflush(stdout, false);
					break;
				case RAW:
					result=(result_t){0,0,0,0,0};
					give_result(stdout);
					break;
				default:
					assert(0);
			}
			return move_next_instruction();
		}
		i++;
	}

	if (prefix_count()>config.max_prefix || 
			(!config.allow_dup_prefix && has_dup_prefix())) {
		switch (output) {
			case TEXT:
				sync_fprintf(stdout, "x: "); print_mc(stdout, 16);
				sync_fprintf(stdout, "... (%s)\n", "prefix violation");
				sync_fflush(stdout, false);
				break;
			case RAW:
				result=(result_t){0,0,0,0,0};
				give_result(stdout);
				break;
			default:
				assert(0);
		}
		return move_next_instruction();
	}

	/* early exit */
	/* check if we are at, or past, the end instruction */
	if (memcmp(inj.i.bytes, search_range.end.bytes, sizeof(inj.i.bytes))>=0) {
		return false;
	}

	/* search based exit */
	switch (mode) {
		case RAND:
			return true;
		case BRUTE:
			return inj.index>=0;
		case TUNNEL:
			return inj.index>=0;
		case DRIVEN:
			return true;
		default:
			assert(0);
	}
}

bool move_next_range(void)
{
	bool result=true;

	switch (mode) {
		case RAND:
		case DRIVEN:
			if (search_range.started) {
				result=false;
			}
			else {
				search_range=total_range;
			}
			break;
		case BRUTE:
		case TUNNEL:
			pthread_mutex_lock(pool_mutex);
			search_range.started=false;
			if (memcmp(range_marker->bytes, total_range.end.bytes,
						sizeof(range_marker->bytes))==0) {
				/* reached end of range */
				result=false;
			}
			else {
				search_range.start=*range_marker;
				search_range.end=*range_marker;
//TODO: there are search bugs here
//#error make sure you don't skip over the first instruction (e.g. 000000...)
//#error there's another error here somewhere...
//somehow take start range from end range..
//len can mostly be taken from range_bytes WHEN YOU MOVE TO A NEW RANGE but
//needs to be from total_range.start/end.len when you are deriving from that
//right now len is set in total_range, and in increment_range for range.end
				if (!increment_range(&search_range.end, config.range_bytes)) {
					/* if increment rolled over, set to end */
					search_range.end=total_range.end;
				}
				else if (memcmp(search_range.end.bytes,
							total_range.end.bytes, sizeof(search_range.end.bytes))>0) {
					/* if increment moved past end, set to end */
					search_range.end=total_range.end;
				}

				*range_marker=search_range.end;
			}
			pthread_mutex_unlock(pool_mutex);
			break;
		default:
			assert(0);
	}

	return result;
}

void zero_insn_end(insn_t* insn, int marker)
{
	int i;
	for (i=marker; i<MAX_INSN_LENGTH; i++) {
		insn->bytes[i]=0;
	}
}

bool increment_range(insn_t* insn, int marker)
{
	int i=marker-1;
	zero_insn_end(insn, marker);

	if (i>=0) {
		insn->bytes[i]++;
		while (insn->bytes[i]==0) {
			i--;
			if (i<0) {
				break;
			}
			insn->bytes[i]++;
		}
	}

	insn->len=marker;

	return i>=0;
}

void initialize_ranges(void)
{
	if (range_marker==NULL) {
		range_marker=mmap(NULL, sizeof *range_marker, 
				PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
		*range_marker=total_range.start;
	}
}

void free_ranges(void)
{
	if (range_marker!=NULL) {
		munmap(range_marker, sizeof *range_marker);
	}
}

void get_rand_insn_in_range(range_t* r)
{
	static uint8_t inclusive_end[MAX_INSN_LENGTH];
	int i;
	bool all_max=true;
	bool all_min=true;

	memcpy(inclusive_end, &r->end.bytes, MAX_INSN_LENGTH);
	i=MAX_INSN_LENGTH-1;
	while (i>=0) {
		inclusive_end[i]--;
		if (inclusive_end[i]!=0xff) {
			break;
		}
		i--;
	}

	for (i=0; i<MAX_INSN_LENGTH; i++) {
		if (all_max && all_min) {
			inj.i.bytes[i]=
				rand()%(inclusive_end[i]-r->start.bytes[i]+1)+r->start.bytes[i];
		}
		else if (all_max) {
			inj.i.bytes[i]=
				rand()%(inclusive_end[i]+1);
		}
		else if (all_min) {
			inj.i.bytes[i]=
				rand()%(256-r->start.bytes[i])+r->start.bytes[i];
		}
		else {
			inj.i.bytes[i]=
				rand()%256;
		}
		all_max=all_max&&(inj.i.bytes[i]==inclusive_end[i]);
		all_min=all_min&&(inj.i.bytes[i]==r->start.bytes[i]);
	}
}
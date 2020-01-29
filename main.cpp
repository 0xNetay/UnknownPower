

void usage(void);
void help(void);
void init_config(int argc, char** argv);
void pin_core(void);
void pretext(void);

int main(int argc, char** argv)
{
	int pid;
	int job=0;
	int i;
	void* packet_buffer_unaligned;
	void* null_p;

	pthread_mutexattr_init(&mutex_attr);
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
	pool_mutex=mmap(NULL, sizeof *pool_mutex, 
			PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	output_mutex=mmap(NULL, sizeof *output_mutex,
			PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	pthread_mutex_init(pool_mutex, &mutex_attr);
	pthread_mutex_init(output_mutex, &mutex_attr);

	init_config(argc, argv);
	pin_core();

	srand(config.seed);

	packet_buffer_unaligned=malloc(PAGE_SIZE*3);
	packet_buffer=(void*)
		(((uintptr_t)packet_buffer_unaligned+(PAGE_SIZE-1))&~(PAGE_SIZE-1));
	assert(!mprotect(packet_buffer,PAGE_SIZE,PROT_READ|PROT_WRITE|PROT_EXEC));
	if (config.nx_support) {
		/* enabling reads and writes on the following page lets us properly
		 * resolve the lengths of some rip-relative instructions that come up
		 * during tunneling: e.g. inc (%rip) - if the next page has no
		 * permissions at all, the fault from this instruction executing is
		 * indistinguishable from the fault of the instruction fetch failing,
		 * preventing correct length determination.  allowing read/write ensures
		 * 'inc (%rip)' can succeed, so that we can find its length. */
		assert(!mprotect(packet_buffer+PAGE_SIZE,PAGE_SIZE,PROT_READ|PROT_WRITE));
	}
	else {
		/* on systems that don't support the no-execute bit, providing
		 * read/write access (like above) is the same as providing execute
		 * access, so this will not work.  on these systems, provide no access
		 * at all - systems without NX will also not support rip-relative
		 * addressing, and (with the proper register initializations) should not
		 * be able to reach the following page during tunneling style fuzzing */
		assert(!mprotect(packet_buffer+PAGE_SIZE,PAGE_SIZE,PROT_NONE));
	}

#if USE_CAPSTONE
	if (cs_open(CS_ARCH_X86, CS_MODE, &capstone_handle) != CS_ERR_OK) {
		exit(1);
	}
	capstone_insn = cs_malloc(capstone_handle);
#endif

	if (config.enable_null_access) {
		null_p=mmap(0, PAGE_SIZE, PROT_READ|PROT_WRITE,
			MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
		if (null_p==MAP_FAILED) {
			printf("null access requires running as root\n");
			exit(1);
		}
	}

	/*
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	*/

	sigaltstack(&ss, 0);

	init_global_ranges();

	for (i=0; i<config.jobs-1; i++) {
		pid=fork();
		assert(pid>=0);
		if (pid==0) {
			break;
		}
		job++;
	}

	while (build_next_range()) {
		/* sync_fprintf(stderr, "job: %d // range: ", job); print_range(stderr, &search_range); sync_fprintf(stderr, "\n");  sync_fflush(stderr,true); */
		while (build_next_instruction()) {
			/* sync_fprintf(stderr, "job: %d // mc: ", job); print_mc(stderr, 16); sync_fprintf(stderr, "\n"); sync_fflush(stderr,true); */
			pretext();
			for (i=1; i<=MAX_INSN_LENGTH; i++) {
				inject(i);
				/* approach 1: examine exception type */
				/* suffers from failure to resolve length when instruction
				 * accesses mapped but protected memory (e.g. writes to .text section) */
				/* si_code = SEGV_ACCERR, SEGV_MAPERR, or undocumented SI_KERNEL */
				/* SI_KERNEL appears with in, out, hlt, various retf, movabs, mov cr, etc */
				/* SI_ACCERR is expected when the instruction fetch fails */
				//if (result.signum!=SIGSEGV || result.si_code!=SEGV_ACCERR) {
				/* approach 2: examine exception address */
				/* correctly resolves instruction length in most foreseeable
				 * situations */
				if (result.addr!=(uint32_t)(uintptr_t)(packet_buffer+PAGE_SIZE)) {
					break;
				}
			}
			result.length=i;
			give_result(stdout);
			tick();
		}
	}

	sync_fflush(stdout, true);
	sync_fflush(stderr, true);

	/* sync_fprintf(stderr, "lazarus!\n"); */

#if USE_CAPSTONE
	cs_free(capstone_insn, 1);
	cs_close(&capstone_handle);
#endif

	if (config.enable_null_access) {
		munmap(null_p, PAGE_SIZE);
	}

	free(packet_buffer_unaligned);

	if (pid!=0) {
		for (i=0; i<config.jobs-1; i++) {
			wait(NULL);
		}
		free_global_ranges();
		pthread_mutex_destroy(pool_mutex);
		pthread_mutex_destroy(output_mutex);
	}

	return 0;
}

void usage(void)
{
	printf("injector [-b|-r|-t|-d] [-R|-T] [-x] [-0] [-D] [-N]\n");
	printf("\t[-s seed] [-B brute_depth] [-P max_prefix]\n");
	printf("\t[-i instruction] [-e instruction]\n");
	printf("\t[-c core] [-X blacklist]\n");
	printf("\t[-j jobs] [-l range_bytes]\n");
}

void help(void)
{
	printf("injector [OPTIONS...]\n");
	printf("\t[-b|-r|-t|-d] ....... mode: brute, random, tunnel, directed (default: tunnel)\n");
	printf("\t[-R|-T] ............. output: raw, text (default: text)\n");
	printf("\t[-x] ................ show tick (default: %d)\n", config.show_tick);
	printf("\t[-0] ................ allow null dereference (requires sudo) (default: %d)\n", config.enable_null_access);
	printf("\t[-D] ................ allow duplicate prefixes (default: %d)\n", config.allow_dup_prefix);
	printf("\t[-N] ................ no nx bit support (default: %d)\n", config.nx_support);
	printf("\t[-s seed] ........... in random search, seed (default: time(0))\n");
	printf("\t[-B brute_depth] .... in brute search, maximum search depth (default: %d)\n", config.brute_depth);
	printf("\t[-P max_prefix] ..... maximum number of prefixes to search (default: %d)\n", config.max_prefix);
	printf("\t[-i instruction] .... instruction at which to start search, inclusive (default: 0)\n");
	printf("\t[-e instruction] .... instruction at which to end search, exclusive (default: ff..ff)\n");
	printf("\t[-c core] ........... core on which to perform search (default: any)\n");
	printf("\t[-X blacklist] ...... blacklist the specified instruction\n");
	printf("\t[-j jobs] ........... number of simultaneous jobs to run (default: %d)\n", config.jobs);
	printf("\t[-l range_bytes] .... number of base instruction bytes in each sub range (default: %d)\n", config.range_bytes);
}

void init_config(int argc, char** argv)
{
	int c, i, j;
	opterr=0;
	bool seed_given=false;
	while ((c=getopt(argc,argv,"?brtdRTx0Ns:DB:P:S:i:e:c:X:j:l:"))!=-1) {
		switch (c) {
			case '?':
				help();
				exit(-1);
				break;
			case 'b':
				mode=BRUTE;
				break;
			case 'r':
				mode=RAND;
				break;
			case 't':
				mode=TUNNEL;
				break;
			case 'd':
				mode=DRIVEN;
				break;
			case 'R':
				output=RAW;
				break;
			case 'T':
				output=TEXT;
				break;
			case 'x':
				config.show_tick=true;
				break;
			case '0':
				config.enable_null_access=true;
				break;
			case 'N':
				config.nx_support=false;
				break;
			case 's':
				sscanf(optarg, "%ld", &config.seed);
				seed_given=true;
				break;
			case 'P':
				sscanf(optarg, "%d", &config.max_prefix);
				break;
			case 'B':
				sscanf(optarg, "%d", &config.brute_depth);
				break;
			case 'D':
				config.allow_dup_prefix=true;
				break;
			case 'i':
				i=0;
				while (optarg[i*2] && optarg[i*2+1] && i<MAX_INSN_LENGTH) {
					unsigned int k;
					sscanf(optarg+i*2, "%02x", &k);
					total_range.start.bytes[i]=k;
					i++;
				}
				total_range.start.len=i;
				while (i<MAX_INSN_LENGTH) {
					total_range.start.bytes[i]=0;
					i++;
				}
				break;
			case 'e':
				i=0;
				while (optarg[i*2] && optarg[i*2+1] && i<MAX_INSN_LENGTH) {
					unsigned int k;
					sscanf(optarg+i*2, "%02x", &k);
					total_range.end.bytes[i]=k;
					i++;
				}
				total_range.end.len=i;
				while (i<MAX_INSN_LENGTH) {
					total_range.end.bytes[i]=0;
					i++;
				}
				break;
			case 'c':
				config.force_core=true;
				sscanf(optarg, "%d", &config.core);
				break;
			case 'X':
				j=0;
				while (opcode_blacklist[j].opcode) {
					j++;
				}
				opcode_blacklist[j].opcode=malloc(strlen(optarg)/2+1);
				assert (opcode_blacklist[j].opcode);
				i=0;
				while (optarg[i*2] && optarg[i*2+1]) {
					unsigned int k;
					sscanf(optarg+i*2, "%02x", &k);
					opcode_blacklist[j].opcode[i]=k;
					i++;
				}
				opcode_blacklist[j].opcode[i]='\0';
				opcode_blacklist[j].reason="user_blacklist";
				opcode_blacklist[++j]=(ignore_op_t){NULL,NULL};
				break;
			case 'j':
				sscanf(optarg, "%d", &config.jobs);
				break;
			case 'l':
				sscanf(optarg, "%d", &config.range_bytes);
				break;
			default:
				usage();
				exit(-1);
		}
	}

	if (optind!=argc) {
		usage();
		exit(1);
	}

	if (!seed_given) {
		config.seed=time(0);
	}
}

void pin_core(void)
{
	if (config.force_core) {
		cpu_set_t mask;
		CPU_ZERO(&mask);
		CPU_SET(config.core,&mask);
		if (sched_setaffinity(0, sizeof(mask), &mask)) {
			printf("error: failed to set cpu\n");
			exit(1);
		}
	}
}

void pretext(void)
{
	/* assistive output for analyzing hangs in text mode */
	if (output==TEXT) {
		sync_fprintf(stdout, "r: ");
		print_mc(stdout, 8);
		sync_fprintf(stdout, "... ");
		#if USE_CAPSTONE
		print_asm(stdout);
		sync_fprintf(stdout, " ");
		#endif
		sync_fflush(stdout, false);
	}
}
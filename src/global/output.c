#include "output.h"

char stdout_buffer[LINE_BUFFER_SIZE*BUFFER_LINES] = { 0 };
char* stdout_buffer_pos = stdout_buffer;
int stdout_sync_counter = 0;

char stderr_buffer[LINE_BUFFER_SIZE*BUFFER_LINES] = { 0 };
char* stderr_buffer_pos = stderr_buffer;
int stderr_sync_counter = 0;

#if USE_CAPSTONE
int print_asm(FILE* f)
{
	if (output==TEXT) {
		uint8_t* code=current_instruction.instruction..bytes;
		size_t code_size=MAX_INSTRUCTION_LENGTH;
		uint64_t address=(uintptr_t)packet_buffer;
	
		if (cs_disasm_iter(
				capstone_handle,
				(const uint8_t**)&code,
				&code_size,
				&address,
				capstone_insn)
			) {
			sync_fprintf(
				f,
				"%10s %-45s (%2d)",
				capstone_insn[0].mnemonic,
				capstone_insn[0].op_str,
				(int)(address-(uintptr_t)packet_buffer)
				);
		}
		else {
			sync_fprintf(
				f,
				"%10s %-45s (%2d)",
				"(unk)",
				" ",
				(int)(address-(uintptr_t)packet_buffer)
				);
		}
		expected_length=(int)(address-(uintptr_t)packet_buffer);
	}

	return 0;
}
#endif

void print_mc(FILE* f, int length)
{
	int i;
	bool p=false;
	if (!is_prefix(current_instruction.instruction.bytes[0])) {
		sync_fprintf(f, " ");
		p=true;
	}
	for (i=0; i<length && i<MAX_INSTRUCTION_LENGTH; i++) {
		sync_fprintf(f, "%02x", current_instruction.instruction.bytes[i]);
		if (
			!p && 
			i<MAX_INSTRUCTION_LENGTH-1 && 
			is_prefix(current_instruction.instruction.bytes[i]) && 
			!is_prefix(current_instruction.instruction.bytes[i+1])
			) {
			sync_fprintf(f, " ");
			p=true;
		}
	}
}

void give_result(FILE* f)
{
	uint8_t* code;
	size_t code_size;
	uint64_t address;
	switch (output) {
		case TEXT:
			switch (mode) {
				case BRUTE:
				case TUNNEL:
				case RAND:
				case DRIVEN:
					sync_fprintf(f, " %s", expected_length==result.length?" ":".");
					sync_fprintf(f, "r: (%2d) ", result.length);
					if (result.signum==SIGILL)  { sync_fprintf(f, "sigill "); }
					if (result.signum==SIGSEGV) { sync_fprintf(f, "sigsegv"); }
					if (result.signum==SIGFPE)  { sync_fprintf(f, "sigfpe "); }
					if (result.signum==SIGBUS)  { sync_fprintf(f, "sigbus "); }
					if (result.signum==SIGTRAP) { sync_fprintf(f, "sigtrap"); }
					sync_fprintf(f, " %3d ", result.si_code);
					sync_fprintf(f, " %08x ", result.addr);
					print_mc(f, result.length);
					sync_fprintf(f, "\n");
					break;
				default:
					assert(0);
			}
			break;
		case RAW:
#if USE_CAPSTONE
			code=inj.i.bytes;
			code_size=MAX_INSN_LENGTH;
			address=(uintptr_t)packet_buffer;
		
			if (cs_disasm_iter(
					capstone_handle,
					(const uint8_t**)&code,
					&code_size,
					&address,
					capstone_insn)
				) {
#if RAW_REPORT_DISAS_MNE 
				strncpy(disas.mne, capstone_insn[0].mnemonic, RAW_DISAS_MNEMONIC_BYTES);
#endif
#if RAW_REPORT_DISAS_OPS
				strncpy(disas.ops, capstone_insn[0].op_str, RAW_DISAS_OP_BYTES);
#endif
#if RAW_REPORT_DISAS_LEN
				disas.len=(int)(address-(uintptr_t)packet_buffer);
#endif
#if RAW_REPORT_DISAS_VAL
				disas.val=true;
#endif
			}
			else {
#if RAW_REPORT_DISAS_MNE 
				strncpy(disas.mne, "(unk)", RAW_DISAS_MNEMONIC_BYTES);
#endif
#if RAW_REPORT_DISAS_OPS
				strncpy(disas.ops, " ", RAW_DISAS_OP_BYTES);
#endif
#if RAW_REPORT_DISAS_LEN
				disas.len=(int)(address-(uintptr_t)packet_buffer);
#endif
#if RAW_REPORT_DISAS_VAL
				disas.val=false;
#endif
			}
#if RAW_REPORT_DISAS_MNE || RAW_REPORT_DISAS_OPS || RAW_REPORT_DISAS_LEN
			sync_fwrite(&disas, sizeof(disas), 1, stdout);
#endif
#endif
			sync_fwrite(inj.i.bytes, RAW_REPORT_INSN_BYTES, 1, stdout);
			sync_fwrite(&result, sizeof(result), 1, stdout);
			/* fflush(stdout); */
			break;
		default:
			assert(0);
	}
	sync_fflush(stdout, false);
}

void sync_fprintf(FILE* f, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	if (f==stdout) {
		stdout_buffer_pos+=vsprintf(stdout_buffer_pos, format, args);
	}
	else if (f==stderr) {
		stderr_buffer_pos+=vsprintf(stderr_buffer_pos, format, args);
	}
	else {
		assert(0);
	}

	va_end(args);
}

void sync_fwrite(const void* ptr, size_t size, size_t count, FILE* f)
{
	if (f==stdout) {
		memcpy(stdout_buffer_pos, ptr, size*count);
		stdout_buffer_pos+=size*count;
	}
	else if (f==stderr) {
		memcpy(stderr_buffer_pos, ptr, size*count);
		stderr_buffer_pos+=size*count;
	}
	else {
		assert(0);
	}
}

void sync_fflush(FILE* f, bool force)
{
#ifdef LINUX
	if (f==stdout) {
		stdout_sync_counter++;
		if (stdout_sync_counter==SYNC_LINES_STDOUT || force) {
			stdout_sync_counter=0;
			pthread_mutex_lock(output_mutex);
			fwrite(stdout_buffer, stdout_buffer_pos-stdout_buffer, 1, f);
			fflush(f);
			pthread_mutex_unlock(output_mutex);
			stdout_buffer_pos=stdout_buffer;
		}
	}
	else if (f==stderr) {
		stderr_sync_counter++;
		if (stderr_sync_counter==SYNC_LINES_STDERR || force) {
			stderr_sync_counter=0;
			pthread_mutex_lock(output_mutex);
			fwrite(stderr_buffer, stderr_buffer_pos-stderr_buffer, 1, f);
			fflush(f);
			pthread_mutex_unlock(output_mutex);
			stderr_buffer_pos=stderr_buffer;
		}
	}
	else {
		assert(0);
	}
#endif
}

void print_insn(FILE* f, instruction_t* insn)
{
	int i;
	for (i=0; i<sizeof(insn->bytes); i++) {
		sync_fprintf(f, "%02x", insn->bytes[i]);
	}
}

void print_range(FILE* f, instruction_range_t* range)
{
	print_insn(f, &range->start);
	sync_fprintf(f, ";");
	print_insn(f, &range->end);
}

void tick(void)
{
	static uint64_t t=0;
	if (config.show_tick) {
		t++;
		if ((t&TICK_MASK)==0) {
			sync_fprintf(stderr, "t: ");
			print_mc(stderr, 8);
			sync_fprintf(stderr, "... ");
			#if USE_CAPSTONE
			print_asm(stderr);
			sync_fprintf(stderr, "\t");
			#endif
			give_result(stderr);
			sync_fflush(stderr, false);
		}
	}
}
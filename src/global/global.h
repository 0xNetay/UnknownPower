// Global Variables
extern void*                packet_buffer;
extern char*                packet;

// Linux Only Variables
#ifdef LINUX
extern char         stack[SIGSTKSZ];
extern stack_t      ss;
// char stack[SIGSTKSZ] = { 0 };
// stack_t ss = { .ss_size = SIGSTKSZ, .ss_sp = stack, };

extern mcontext_t   fault_context;

extern pthread_mutex_t* pool_mutex;

extern pthread_mutexattr_t mutex_attr;

extern char debug, resume, preamble_start, preamble_end;
void preamble();



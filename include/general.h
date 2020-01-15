#pragma once

#define _GNU_SOURCE

// Global Includes
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <assert.h>

// #define LINUX

// Linux Include
#ifdef LINUX
#include <unistd.h>
#include <execinfo.h>
#include <ucontext.h>
#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <sys/wait.h>
#endif

#define STR(x) #x
#define XSTR(x) STR(x)

#define UD2_SIZE  2
#define PAGE_SIZE 4096
#define TF        0x100
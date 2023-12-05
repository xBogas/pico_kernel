#ifndef W_KERNEL_THREAD_H
#define W_KERNEL_THREAD_H

#include "memory.h"

#define MIN_STACK_SIZE 			PAGE_SIZE
#define OFFSET_THREAD_ENTRY		20u

enum th_state {
	Ready,
	Running,
	Waiting,
	Blocked,
	Stopped,
	Killed
};

enum th_prio {
	prio_low,
	prio_def,
	prio_high,
	prio_max
};

struct thread_attr {
	const char	*name;
	uint16_t	priority;
	uint16_t	stack_size;
};

//! Changing stack_ptr off will break context switch
//! Export stack_ptr offset
struct thread_handle {
	uint16_t id;
	uint16_t priority;
	const char *name;
	uint16_t state;
	uint16_t stack_size; // not needed
	uint32_t stack_ptr;
	uint32_t stack_top;	 // size is this addr to its value
};

int thread_create(void (*thread_fn)(void *data),
					void *data,
					struct thread_attr *attr);

//TODO:

// validate thread priority
static uint16_t check_prio(uint16_t prio)
{
	// trim value 1 - MAX_PRIO
	return prio;
}

#endif
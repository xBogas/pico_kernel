#ifndef W_KERNEL_THREAD_H
#define W_KERNEL_THREAD_H

#include "memory.h"

#define MIN_STACK_SIZE 			PAGE_SIZE

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

//! Set stack ptr to first 4 bytes
struct thread_handle {
	uint32_t stack_ptr;
	void *obj;				// obj to sleep on
	uint16_t id;
	uint16_t priority;
	const char *name;
	uint16_t state;
	uint16_t stack_size; 	// not needed
	uint32_t stack_top;		// size is this addr to its value
};

int thread_create(void (*thread_fn)(void *data),
					void *data,
					struct thread_attr *attr);

void bs_wait(uint32_t ms);

void wait(uint32_t ms);

//TODO:

// validate thread priority
static uint16_t check_prio(uint16_t prio)
{
	// trim value 1 - MAX_PRIO
	return prio;
}

#endif
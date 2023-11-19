#ifndef W_KERNEL_THREAD_H
#define W_KERNEL_THREAD_H

#include "memory.h"

enum thread_state {
	Ready,
	Running,
	Stopped,
	Blocked
};

struct thread_attr {
	const char	*name;
	uint16_t	priority;
	uint16_t	stack_size;
};

struct thread_handle {
	void (*thread_fn)(void *data);
	void *data;
	uint16_t id;
	uint16_t priority;
	char *name;
	uint32_t *ptr_stack;
	uint32_t *top_stack; 
	uint32_t stack_size;
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
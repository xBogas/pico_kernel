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
	Killed,
	Idle
};

enum th_prio {
	prio_idle,
	prio_low,
	prio_def,
	prio_high,
	prio_max
};

// thread attributes
struct thread_attr {
	const char	*name;
	uint16_t	priority;
	uint16_t	stack_size;
};

// thread handle stack_ptr must be the first variable
// so that when context switch is performed
// no offset is required to get the stack_ptr
// dereferencing the thread handle will return the stack_ptr
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

/**
 * @brief Create a thread object.
 * 
 * @param thread_fn thread function
 * @param data param to pass to thread function
 * @param attr thread attributes
 * @return int thread id
 */
int thread_create(void (*thread_fn)(void *data),
					void *data,
					struct thread_attr *attr);

/**
 * @brief Busy wait for ms
 * 
 * @param ms miliseconds
 */
void bs_wait(uint32_t ms);

/**
 * @brief Send thread to sleep for ms
 * 
 * @param ms miliseconds
 */
void wait(uint32_t ms);


//TODO:
// validate thread priority
static uint16_t check_prio(uint16_t prio)
{
	// trim value 1 - MAX_PRIO
	return prio;
}

#endif
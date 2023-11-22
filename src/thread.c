#include "thread.h"
#include "scheduler.h"

#include <string.h>
#include <stdio.h>

static uint32_t thread_id = 0;

#define MIN_STACK_SIZE 128

/**
 * ? Allocate stack-handle first
 * Allocate handle
 * Allocate stack
 * Initialize thread
 * Add to scheduler
 * Should wake up new thread?
 * TODO: Config max size thread name
 */
int thread_create(void (*thread_fn)(void *data), void *data, struct thread_attr *attr) {
	if (thread_fn == NULL)
		return 0;

	struct thread_handle *h_thread = malloc(sizeof(struct thread_handle));
	if (!h_thread)
		return 0;

	uint32_t stack = MIN_STACK_SIZE;
	h_thread->thread_fn = thread_fn;
	h_thread->data	= data;
	h_thread->id = ++thread_id;

	if (attr != NULL) {
		h_thread->priority = check_prio(attr->priority);

		h_thread->name = malloc(strlen(attr->name));
		strcpy(h_thread->name, attr->name);

		if (attr->stack_size > MIN_STACK_SIZE)
			stack = attr->stack_size;
	}
	else {
		h_thread->priority = 1;
		h_thread->name = NULL;
	}

	// check if stack size is correct -> 32 bit ?
	h_thread->ptr_stack = malloc(stack * sizeof(uint32_t));
	if (h_thread->ptr_stack == NULL) {
		k_free(h_thread);
		return 0;
	}
	memset(h_thread->ptr_stack, 0, stack * sizeof(uint32_t));
	// get top address of stack
	h_thread->top_stack = &( h_thread->ptr_stack[stack-1]);
	printf("top stack 0x%p\n", h_thread->top_stack);
	h_thread->stack_size = stack;

	//TODO: Continue thread handle initialization

	if (!sched_add_thread(h_thread))
		return 0;

	//? when should wake up thread
	// if (if priority bigger)
	// 	wake_up_thread(h_thread);

	return thread_id;
}

void thread_kill(uint32_t thread_id)
{
	thread_id--;
}
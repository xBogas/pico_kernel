#ifndef W_KERNEL_SCHEDULER_H
#define W_KERNEL_SCHEDULER_H

// Use process stack pointer for user threads
#define PRIV_TH 		0x02
#define UNPRIV_TH 		0x03

#include "thread.h"
#include "terminal.h"
#include "locks.h"

// initialize scheduler
void sched_init(void);

// add thread_handle
uint32_t sched_add_thread(struct thread_handle* th);

// start the scheduler
// for now just loop through the threads
void start_sched(void);

// get current thread
static __force_inline struct thread_handle *mythread(void)
{
	uint32_t ptr;
#ifdef USE_PRIV_THREADS
	ptr = __get_PSP();
#else
	if (__get_current_exception())
		ptr = __get_PSP();
	else // assume user mode using psp
		__asm volatile("mov %0, sp\n" : "=r"(ptr));
#endif
	return (struct thread_handle *)(PG_ROUND_DOWN(ptr));
}

// sleep thread
void sleep(struct mutex *mtx);

// wake up threads sleeping on mutex
void wake_up(struct mutex *mtx);


//TODO:
// check if sched started running 
int sched_runtime(void);
#endif
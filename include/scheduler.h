#ifndef W_KERNEL_SCHEDULER_H
#define W_KERNEL_SCHEDULER_H

#include "thread.h"
#include "terminal.h"

#define ERROR(str)		\
	{	printk(str);	\
		exit(1);		\
	}		

// initialize scheduler
void sched_init(void);

// add thread_handle
uint32_t sched_add_thread(struct thread_handle* th);

// start the scheduler
// for now just loop through the threads
void start_sched(void);

//TODO:
// check if sched started running 
int sched_runtime(void);

// wake up thread
void wake_up_thread(struct thread_handle* th);

#endif
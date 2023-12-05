#ifndef W_KERNEL_SCHEDULER_H
#define W_KERNEL_SCHEDULER_H

#include "thread.h"
#include "terminal.h"

#define ERROR(str)		\
	{	printk(str);	\
		exit(1);		\
	}		

// add thread_handle
uint32_t sched_add_thread(struct thread_handle* th);

// start the scheduler
// for now just loop through the threads
void start_sched(void);


// check if sched started running 
int sched_runtime(void);

// not sure how to do this for now
// this should wake up thread
void wake_up_thread(struct thread_handle* th);

#endif
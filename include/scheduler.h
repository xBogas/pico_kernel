#ifndef W_KERNEL_SCHEDULER_H
#define W_KERNEL_SCHEDULER_H

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
struct thread_handle *mythread(void);

// sleep thread
void sleep(struct mutex *mtx);

// wake up threads sleeping on mutex
void wake_up(struct mutex *mtx);


//TODO:
// check if sched started running 
int sched_runtime(void);
#endif
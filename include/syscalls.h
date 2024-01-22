#ifndef W_KERNEL_SYSCALLS_H
#define W_KERNEL_SYSCALLS_H

#define MAX_SYSCALL 1

#ifndef __ASSEMBLER__

#include "kernel.h"

/**
 * @brief SVC call to perform 
 * context switch ID#0
 * 
 * Pass stack pointer of current thread and next thread
 * (struct thread_switch)
 */
void sys_switch(void *);

/**
 * @brief SVC call to perform 
 * initial thread jump ID#1
 * 
 */
void sys_sched(uint32_t);

#endif

#endif
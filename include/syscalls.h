#ifndef W_KERNEL_SYSCALLS_H
#define W_KERNEL_SYSCALLS_H

#define MAX_SYSCALL 1

#ifndef __ASSEMBLER__

#include "kernel.h"
#include "locks.h"


/**
 * @brief process syscall
 * 
 * @param args register params
 */
void syscall_handler(uint32_t *args);


void sys_switch(void *);
void sys_sched(uint32_t);

#endif

#endif
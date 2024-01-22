#include "syscalls.h"



void sys_switch(void *param)
{
	__asm ("mov r0, %0\n" : : "r" (param));
	__asm ("svc #0\n");
}

void sys_sched(uint32_t sp)
{
	__asm ("mov r0, %0\n" : : "r" (sp));
	__asm ("svc #1\n");
}
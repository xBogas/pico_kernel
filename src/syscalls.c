#include "syscalls.h"
#include "locks.h"
#include "scheduler.h"

extern void context_switch(void *ptr);

// args vector: 0x20001094 -> 0x2003f400
// r0 - args[0]
// r1 - args[1]
// r2 - args[2]
// r3 - args[3]
// r12 - args[4]
// lr - args[5]
// pc - args[6] 0x100011fa
// xPSR - args[7]
void __attribute__((naked)) syscall_handler_c(uint32_t *args) // 0x20001094
{
	__asm("push {lr}\n");

	void *ptr = (void *)args[0];
	ptr++;
	uint32_t var = *((uint32_t *)ptr);
	printk("syscall_handler_c: %x\n", var);
	// switch (id)
	// {
	// case 0:
	// 	ptr = (void *)args[0];
	// 	//context_switch((void *)args[0]);
	// 	break;

	// // case 1:
	// // 	sleep((struct mutex *)args[0]);
	// // 	break;

	// // case 2:
	// // 	wake_up((struct mutex *)args[0]);
	// // 	break;

	// default:
	// 	panic("unknown syscall %d\n", id);
	// 	break;
	// }
	__asm("pop {pc}\n");
}


void sys_switch(void *param) // 0x20001090
{
	//syscall_handler_c(param);
	__asm ("mov r0, %0\n" : : "r" (param));
	__asm ("svc #0\n");
}

void sys_sched(uint32_t sp)
{
	__asm ("mov r0, %0\n" : : "r" (sp));
	__asm ("svc #1\n");
}
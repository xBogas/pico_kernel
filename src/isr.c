#include "RP2040.h"
#include <stdlib.h>
#include "terminal.h"
#include "kernel.h"

void isr_invalid(void)
{
	panic("NVIC invalid called\n");
}

void isr_nmi(void)
{
	panic("NVIC nmi called\n");
}

struct stack_frame {
	volatile uint32_t *r0;
	volatile uint32_t *r1;
	volatile uint32_t *r2;
	volatile uint32_t *r3;
	volatile uint32_t *r12;
	volatile uint32_t *lr;
	volatile uint32_t *pc;
	volatile uint32_t *x_psr;
};

void hardfault_handler(struct stack_frame *frame)
{
	printk("sp    %x\n", *frame);
	printk("r0:   %x\n", frame->r0);
	printk("r1:   %x\n", frame->r1);
	printk("r2:   %x\n", frame->r2);
	printk("r3:   %x\n", frame->r3);
	printk("r12:  %x\n", frame->r12);
	printk("lr:   %x\n", frame->lr);
	printk("pc:   %x\n", frame->pc);
	printk("xpsr: %x\n", frame->x_psr);

	printk("Error at instruction: %x\n", frame->pc);
	while (1)
	{ }
	// if user mode
	// do cleanup of process that caused the fault
	// get thread with page alignment
}

void isr_pendsv(void)
{
	panic("NVIC pendsv called\n");
}
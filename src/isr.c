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
	// panic_terminal();
	// for (size_t i = 500; i != 0; i--)
	// 	;

	// force_printk("sp    %x\n", *frame);
	// force_printk("r0:   %x\n", frame->r0);
	// force_printk("r1:   %x\n", frame->r1);
	// force_printk("r2:   %x\n", frame->r2);
	// force_printk("r3:   %x\n", frame->r3);
	// force_printk("r12:  %x\n", frame->r12);
	// force_printk("lr:   %x\n", frame->lr);
	// force_printk("pc:   %x\n", frame->pc);
	// force_printk("xpsr: %x\n", frame->x_psr);

	// force_printk("Error at instruction: %x\n", frame->pc);
	while(1)
		__breakpoint();
	// if user mode
	// do cleanup of process that caused the fault
	// get thread with page alignment
}
.syntax unified
.cpu cortex-m0plus

#include "syscalls.h"

//TODO: 
// check for stack corruption (valid addr func)
// check if EXC_RETURN is valid
// check xPSR T bit

// lr will have the value of EXC_RETURN
// if 0xFFFFFFF9UL -> using msp
// then lr+7 == 0 (overflow)
.thumb_func
.global isr_hardfault
.type isr_hardfault, %function
isr_hardfault:
	mov		r3, lr
	adds	r3, #7 	
	bne.n	stack_is_psp
	mrs		r0, msp
enter_c:
	ldr		r2, =hardfault_handler
	bx 		r2

stack_is_psp:
	mrs		r0, psp
	b		enter_c


.thumb_func
.global isr_svcall
.type isr_svcall, %function
isr_svcall:
	movs	r0, #4
	mov		r1, lr
	cmp		r1, r0
	beq		stack_used_msp
	mrs		r0, psp
	ldr		r1, =syscall_handler
	bx 		r1

stack_used_msp:
	mrs		r0, msp
	ldr		r1, =syscall_handler
	bx		r1

.extern context_switch

.thumb_func
.global syscall_handler
.type syscall_handler, %function
syscall_handler:
	ldr		r1, [r0, #24]			// get instruction address of svc
	subs	r1, #2					// go back two bytes
	ldrb	r1, [r1, #0]			// load byte (svc number)
    cmp 	r1, MAX_SYSCALL				// check if valid syscall
	bgt 	invalid_syscall
	ldr 	r0, [r0, #0]			// get r0 from previous svc call
	bl		context_switch

invalid_syscall:
	ldr 	r0, =isr_hardfault
	bx 		r0



.extern systick_counter
.extern sched_status
.extern isr_systick_12
.extern sched

.syntax unified
.global isr_systick
.type isr_systick, %function
isr_systick:
	ldr		r0, =systick_counter
	ldr		r1, [r0, #0]
	adds	r1, #1					// increment systick counter
	cmp		r1, #10					// check if 10 ticks have passed
	bgt		prepare_swap
	str		r1, [r0, #0]			// store value
	bx		lr


prepare_swap:
	movs	r1, #0
	str		r1, [r0, #0]			// reset systick counter
	mrs		r3, psp					// get stack pointer
	ldr 	r1, =0xfffffc00			// ~(PAGE_SIZE - 1)
	ands	r3, r3, r1				// round down to nearest page
	ldr 	r0, =sched_status
	str 	r3, [r0, #0]			// set sched_status.run to current thread sp
	ldr 	r3, =sched
	adds	r3, #16					// get sched.first address
	ldr 	r2, [r3, #0]			// get sched.first->th address
	ldr 	r1, [r2, #0]			// get sched.first->th value
	str 	r1, [r0, #4] 			// set sched_status.next to sched.first->th
	ldr 	r2, [r2, #4]			// get sched.first->next value	
	str 	r2, [r3] 				// set sched.first = sched.first->next
	bl 		context_switch

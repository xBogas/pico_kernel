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
	mov		r3, lr
	adds	r3, #7
	cmp		r3, #0 	
	beq		stack_used_msp
	mrs		r0, psp
	ldr		r1, =syscall_handler
	bx 		r1

stack_used_msp:
	mrs		r0, msp
	ldr		r1, =syscall_handler
	bx		r1

// .extern context_switch
// .extern sched_jump

.thumb_func
.global syscall_handler
.type syscall_handler, %function
syscall_handler:
	ldr		r1, [r0, #24]			// get instruction address of svc
	subs	r1, #2					// go back two bytes
	ldrb	r1, [r1, #0]			// load byte (svc number)
    cmp 	r1, MAX_SYSCALL			// check if valid syscall
	bgt 	invalid_syscall			// svc call > MAX_SYSCALL
	beq 	syscall_1				// svc call == 1 -> sched_jump
	//cmp 	r1, #0
	ldr 	r0, [r0, #0]			// get r0 from previous svc call
	bl		context_switch

syscall_1:
	ldr 	r0, [r0, #0]			// get r0 from previous svc cal
	bl 		sched_jump

invalid_syscall:
	ldr 	r0, =isr_hardfault
	bx 		r0



.extern systick_counter
.extern pre_switch

.syntax unified
.global isr_systick
.type isr_systick, %function
isr_systick:
@ 	ldr		r0, =systick_counter
@ 	ldr		r1, [r0, #0]
@ 	adds	r1, #1					// increment systick counter
@ 	cmp		r1, #10					// check if 10 ticks have passed
@ 	bgt		prepare_swap
@ 	str		r1, [r0, #0]			// store value
@ 	bx		lr

@ prepare_swap:
@ 	movs	r1, #0
@ 	str		r1, [r0, #0]			// reset systick counter
	mrs		r3, psp					// get stack pointer
	ldr 	r1, =0xfffffc00			// ~(PAGE_SIZE - 1)
	ands	r3, r3, r1				// round down to nearest page
	mov 	r0, r3
	bl 		pre_switch				// call pre_switch
	cmp		r0, #0					// check if context switch is needed
	bne		do_switch
	ldr		r0, =0xFFFFFFFD // EXC_RETURN_THREAD
	bx		r0

do_switch:
	bl 		context_switch

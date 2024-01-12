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

.extern sched_core1
.extern sched_core0

.syntax unified
.global isr_pendsv
.type isr_pendsv, %function
isr_pendsv:
	ldr 	r0, =0xd0000000			// read cpu id
	ldr 	r1, [r0, #0]
	cmp 	r1, #0
	beq 	swap_core_0
swap_core_1:
	ldr 	r0, =sched_core1
	ldr 	r1, =context_switch
	bx 		r1

swap_core_0:
	ldr 	r0, =sched_core0
	ldr 	r1, =context_switch
	bx 		r1

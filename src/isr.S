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
    cmp r1, MAX_SYSCALL				// check if valid syscall
	bgt 	invalid_syscall
	ldr 	r0, [r0, #0]			// get r0 from previous svc call
	bl		context_switch

invalid_syscall:
	ldr 	r0, =isr_hardfault
	bx 		r0

.syntax unified
.cpu cortex-m0plus

.thumb_func
.global isr_hardfault
.type isr_hardfault, %function

//TODO: 
// check for stack corruption (valid addr func)
// check if EXC_RETURN is valid
// check xPSR T bit

// lr will have the value of EXC_RETURN
// if 0xFFFFFFF9UL -> using msp
// then lr+7 == 0 (overflow)
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


.extern syscall_handler

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
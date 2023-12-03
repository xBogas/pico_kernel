.syntax unified
.cpu cortex-m0plus
.thumb

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

// this is not likely to happen (for now at least)
// when enter user code this should be reversed
stack_is_psp:
	mrs		r0, psp
	b		enter_c

/*
TODO: check if EXC_RETURN is 0xFFFFFFF1UL 
handler mode ! (called from isr)
stack_is_msp:
	mrs		r0, msp
	b		enter_c
*/

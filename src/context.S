.syntax unified
.cpu cortex-m0plus
.thumb

.thumb_func
.global context_switch
.type context_switch, %function
context_switch: // do random stuff
	cpsid	i
	ldmia	r0!, {r1, r2}	// load thread run, and next

// save context // TODO: check for stack overflow
	mrs		r0, psp			// get sp
	subs	r0, r0, #32		// add space for R4 ... R11
	str		r0, [r1, #12]	// save sp // TODO: Export SP offset

	stmia	r0!, {r4-r7}	// save registers r4 .. r7
	mov		r4, r8			// move registers to be saved
	mov		r5, r9
	mov		r6, r10
	mov		r7, r11
	stmia	r0!, {r4-r7}	// save registers r8 .. r11

// At this point r2 holds thread to run
	adds	r2, r2, #12
	ldr		r0, [r2]		// load new stack pointer

	ldmia	r0!,{r4-r7}		// load all registers		
	mov		r8, r4
	mov		r9, r5
	mov 	r10, r6
	mov		r11, r7
	ldmia	r0!, {r4-r7}
	msr		psp, r0

	ldr		r0, =0xFFFFFFFD // EXC_RETURN_THREAD
	cpsie	i
	bx		r0				// exit interrupt

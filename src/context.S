.syntax unified
.cpu cortex-m0plus
.thumb

.thumb_func
.global context_switch
.type context_switch, %function
context_switch:
	cpsid	i
	ldmia	r0!, {r1, r2}	// load thread run, and next

// save context // TODO: check for stack overflow
	mrs		r0, psp			// get sp
	subs	r0, r0, #32		// add space for R4 ... R11
	str		r0, [r1]		// save sp

	stmia	r0!, {r4-r7}	// save registers r4 .. r7
	mov		r4, r8			// move registers to be saved
	mov		r5, r9
	mov		r6, r10
	mov		r7, r11
	stmia	r0!, {r4-r7}	// save registers r8 .. r11

// At this point r2 holds thread to run
	ldr		r0, [r2]		// load new stack pointer
	adds	r0, r0, #16		// adjust addr
	ldmia	r0!,{r4-r7}		// restore registers r8 .. r11	
	mov		r8, r4
	mov		r9, r5
	mov 	r10, r6
	mov		r11, r7
	msr		psp, r0
	subs	r0, r0, #32		// adjust addr
	ldmia	r0!, {r4-r7}	// restore registers r4 .. r7

	ldr		r0, =0xFFFFFFFD // EXC_RETURN_THREAD
	cpsie	i
	bx		r0				// exit interrupt


// must ignore current state of registers and execute a jump
// r0 = thread to run stack pointer
.thumb_func
.global sched_jump
.type sched_jump, %function
sched_jump:
	adds	r0, r0, #16		// adjust addr
	ldmia	r0!,{r4-r7}		// restore registers r8 .. r11	
	mov		r8, r4
	mov		r9, r5
	mov 	r10, r6
	mov		r11, r7
	msr		psp, r0
	subs	r0, r0, #32		// adjust addr
	ldmia	r0!, {r4-r7}	// restore registers r4 .. r7
	ldr		r0, =0xFFFFFFFD // EXC_RETURN_THREAD
	bx		r0				// exit interrupt

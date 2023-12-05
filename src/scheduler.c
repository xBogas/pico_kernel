#include "scheduler.h"
#include "locks.h"

struct next {
	struct thread_handle* th;
	struct next *next;
};


struct {
	struct spinlock lock;
	struct next *first;
	struct next *last;
	uint32_t id;
} sched;

void init_sched(void)
{
	init_lock(&sched.lock, "scheduler");
}

static uint32_t new_thread_id(void)
{
	uint32_t id;

	acquire_lock(&sched.lock);
	id = sched.id++;
	release_lock(&sched.lock);

	return id;
}


//! just adding in the end for now
uint32_t sched_add_thread(struct thread_handle* th)
{
	struct next *tmp = k_malloc(); // TODO: change from page allocation
	tmp->th = th;
	uint32_t id;

	acquire_lock(&sched.lock);
	id = sched.id++;
	if (unlikely(!sched.first)) {
		sched.first = tmp;
		tmp->next = tmp;
		sched.last = tmp;
	} else {
		tmp->next = sched.last->next;
		sched.last->next = tmp;
		sched.last = tmp;
	}
	release_lock(&sched.lock);

	return id;
}


#include "hardware/clocks.h"
#include "hardware/structs/systick.h"
#include "RP2040.h"
#include "core_cm0plus.h"

#define SYSTEM_CLOCK 	125000000UL
#define SYS_TICK		SYSTEM_CLOCK/1000 -1

void print_thread_info(void);

void __attribute((naked)) start_sched(void)
{
	NVIC_SetPriority(PendSV_IRQn, 0xff);
	NVIC_SetPriority(SysTick_IRQn, 0x00);

	systick_hw->csr = 0;
	systick_hw->rvr = SYS_TICK;// 0x00ffffff;
	systick_hw->cvr = 0;
	systick_hw->csr = 0b0111;

	printk("systick set to %x\n", systick_hw->rvr);
	printk("systick set to %x\n", SYS_TICK);
	__enable_irq(); //? it should already be enabled

	struct thread_handle *run = sched.first->th;
	__set_PSP(run->stack_ptr);// offset for thread entry
	__set_CONTROL(0x03);
	__ISB();

	print_thread_info();

	// consume stack frame to get pc
	__asm (
		"pop {r0, r1, r2, r3}\n"
		"mov r8,  r0\n"
		"mov r9,  r1\n"
		"mov r10, r2\n"
		"mov r11, r3\n"
		"pop {r4, r5, r6, r7}\n"
		"pop {r0, r1, r2, r3}\n"
		"pop {r4, r5}\n"
		"pop {pc}\n"
	);
}

void print_thread_info(void)
{
	const struct next *iter = sched.first;

	while (iter != sched.last)
	{
		printk("thread id 	%d\n", iter->th->id);
		printk("thread name %s\n", iter->th->name);
		printk("thread top	%x\n", iter->th->stack_top);
		iter = iter->next;
	}
}


extern void context_switch(void *ptr);

struct {
	struct thread_handle *run;
	struct thread_handle *next;
}sched_status;

uint32_t systick_counter;

static void log_ticks(void)
{
	static uint32_t var = 13;
	var++;
}

void isr_systick(void)
{
	systick_counter++;
	if (systick_counter > 1000) {
		systick_counter = 0;
		printk("switching context\n");

		sched_status.run = sched.first->th;
		sched_status.next = sched.first->next->th;
		sched.first = sched.first->next;
		context_switch(&sched_status);
	}
}
// check if sched started running 
int sched_runtime(void)
{
	return 0;
}

// not sure how to do this for now
// this should wake up thread
void wake_up_thread(struct thread_handle* th)
{
	// wait for th to start running
}
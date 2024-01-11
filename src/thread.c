#include "thread.h"
#include "scheduler.h"
#include "memory.h"
#include "locks.h"


struct stack_frame {
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
};


// TODO:
static void thread_exit(struct thread_handle *th)
{
	printk("performing cleanup for thread\n");
	free(th);
	return;
}

static void thread_entry(void (*entry)(void *), void *arg, struct thread_handle *th)
{
	if (!entry || !th)
		panic("invalid thread_entry params\n");

	th->state = Running;
	printk("starting thread %s on core%d\n", th->name, cpu_id());
	entry(arg);
	printk("terminating thread %s\n", th->name);
	thread_exit(th);
}

/**
 * This is only for memory allocation without mpu
*/
int thread_create(void (*thread_fn)(void *data), void *data, struct thread_attr *attr) {
	if (!thread_fn)
		return -1;

	const char *name = NULL;
	uint16_t prio = prio_def;
	// uint16_t stack_size = PAGE_SIZE - sizeof(struct thread_handle);

	if (attr) {
		prio = check_prio(attr->priority);
		name = attr->name;
		//stack_size = MAX(attr->stack_size, stack_size);
	}

	void *start = k_alloc();
	if(!start)
		panic("no memory for more threads\n");

	void *top = start + PAGE_SIZE;


	struct thread_handle *th = (struct thread_handle *)start;
	th->priority = prio;
	th->state = Ready;
	th->name = name;
	th->stack_top = (uint32_t)top;
	th->stack_ptr = th->stack_top - sizeof(struct stack_frame);

	// for debug
	static struct stack_frame *frame;
	frame = (struct stack_frame *)(top - sizeof(struct stack_frame));
	frame->r8 	= 8;
	frame->r9 	= 9;
	frame->r10 	= 10;
	frame->r11	= 11;
	frame->r4 	= 4;
	frame->r5 	= 5;
	frame->r6 	= 6;
	frame->r7 	= 7;
	frame->r0 	= (uint32_t)thread_fn;
	frame->r1 	= (uint32_t)data;
	frame->r2 	= (uint32_t)th;
	frame->r3 	= 3;
	frame->r12 	= 12;
	frame->lr	= 0;
	frame->pc	= (uint32_t)thread_entry;
	frame->psr	= 0x01000000u; // set xPSR for cortex M

	th->id = sched_add_thread(th);
	return th->id;
}

#include "hardware/timer.h"
#include "scheduler.h"

void bs_wait(uint32_t ms)
{
	uint32_t start = time_us_32();
	while (time_us_32() - start < ms*1000)
	{ }
}

void wait(uint32_t ms)
{
	struct thread_handle *th = mythread();
	yield(th, ms);
}

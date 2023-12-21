#include "scheduler.h"
#include "locks.h"
#include "syscalls.h"

#include "hardware/structs/systick.h"


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


void sched_init(void)
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


// ask scheduler for next thread to run!
static struct thread_handle *get_runnable(void)
{
	struct next *iter = sched.first;
	do
	{
		if (iter->th->state == Ready)
			return iter->th;

		iter = iter->next;
	} while (iter != sched.first || iter != NULL);

	return NULL;	
}


static void preempt(struct thread_handle *next, struct thread_handle *running)
{
	if( next->priority < running->priority)
		next->priority = running->priority;
}


//! just adding in the end for now
uint32_t sched_add_thread(struct thread_handle* th)
{
	struct next *tmp = malloc(sizeof(struct next));
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


#define SYSTEM_CLOCK 	125000000UL
#define SYS_TICK		SYSTEM_CLOCK/1000 -1

extern void context_switch(void *ptr);
void __attribute__((naked)) isr_systick(void);
void start_sched(void);

void start_sched(void)
{
	NVIC_SetPriority(PendSV_IRQn, 0xff);
	NVIC_SetPriority(SysTick_IRQn, 0x00);

	systick_hw->csr = 0;
	systick_hw->rvr = SYS_TICK;
	systick_hw->cvr = 0;
	systick_hw->csr = 0b0111;

	acquire_lock(&sched.lock);

	struct thread_handle *run = sched.first->th;
	sched.first = sched.first->next;
	__set_PSP(run->stack_ptr);// offset for thread entry

	release_lock(&sched.lock);

	__set_CONTROL(0x03);
	__ISB();

	// consume stack frame to get pc
	__asm (
		"pop {r0, r1, r2, r3}\n"
		"mov r8,  r0\n"
		"mov r9,  r1\n"
		"mov r10, r2\n"
		"mov r11, r3\n"
		"pop {r4, r5, r6, r7}\n"
		"pop {r0, r1, r2, r3}\n"
		"pop {r4, r5}\n" // r12, lr
		"pop {pc}\n" // entry point
	);
}


struct {
	struct thread_handle *run;
	struct thread_handle *next;
}sched_status;


static uint32_t systick_counter = 0;

void isr_systick(void)
{
	__asm ("push {lr}\n");

	systick_counter++;
	if (systick_counter > 10) {
		systick_counter = 0;

		sched_status.run = mythread();
		sched_status.next = sched.first->th;
		sched.first = sched.first->next;
		printk("isr %s -> %s\n", sched_status.run->name, sched_status.next->name);
		if (sched_status.run->id != sched_status.next->id)
			context_switch(&sched_status);
	}

	__asm ("pop {pc}\n");
}


// check if sched started running 
int sched_runtime(void)
{
	return 0;
}


static void scheduler(struct thread_handle *running, struct thread_handle *next)
{
	// don't schedule threads
	systick_counter = 0;

	if (unlikely(running->state == Running))
		panic("scheduler called on - thread running");

	if (unlikely(running != mythread()))
		panic("thread mismatch");
	
	//TODO: scheduling algorithm
	sched_status.run = running;
	sched_status.next = next;

	preempt(next, running);

	printk("reschedule %s\n", running->name);
	sys_switch(&sched_status);
	printk("wake up %s\n", running->name);
}


struct thread_handle *mythread(void)
{
	uint32_t ptr;
	if (__get_current_exception())
		ptr = __get_PSP();
	else // assume user mode using psp
		__asm volatile("mov %0, sp\n" : "=r"(ptr));

	return (struct thread_handle *)(PG_ROUND_DOWN(ptr));
}


static void add_blocked(struct mutex *mtx, struct thread_handle *th)
{
	struct next *lst = malloc(sizeof(struct next));
	lst->th = th;
	lst->next = mtx->blocked;
	mtx->blocked = lst;

	// for debug
	// if (mtx->blocked)
	// 	return;

	// mtx->blocked = th;
}


void sleep(struct mutex *mtx)
{
	struct thread_handle *th = mythread();
	printk("sending thread to sleep %s\n", th->name);

	th->state = Blocked;
	th->obj = mtx;
	add_blocked(mtx, th);

	struct spinlock *lk = &mtx->sp_lk;
	release_lock(lk); // 0xd000016c

	scheduler(th, mtx->owner); // must reschedule mutex owner

	th->obj = 0;

	if (holding(lk))
		printk("mutex sp lck %s is held\n", lk->name);

	acquire_lock(&mtx->sp_lk);
}


static struct thread_handle *get_th(struct mutex *mtx)
{
	struct next *lst = mtx->blocked;
	mtx->blocked = lst->next;
	struct thread_handle *th = lst->th;
	free(lst);

	return th;
}


// wake up threads sleeping on mutex
void wake_up(struct mutex *mtx)
{
	if (!mtx->blocked)
		return;

#if DEBUG_MTX_BLOCKED
	printk("Sleeping on mutex: %s\n", mtx->sp_lk.name);
	struct next *iter = mtx->blocked;
	while (iter != NULL) {
		printk("thread: %s\n", iter->th->name);
		iter = iter->next;
	}
#endif

	struct thread_handle *th = get_th(mtx);
	printk("waking up %s\n", th->name);

	//TODO: scheduling algorithm
	sched_status.run = mythread();
	sched_status.next = th;
	while (sched.first->th != th)
		sched.first = sched.first->next;	

	mtx->blocked = NULL;

	release_lock(&mtx->sp_lk);

	if (holding(&mtx->sp_lk))
		printk("mutex sp lck %s is held\n", mtx->sp_lk.name);

	sys_switch(&sched_status);

	acquire_lock(&mtx->sp_lk);
}

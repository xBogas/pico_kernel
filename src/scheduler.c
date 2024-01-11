#include "scheduler.h"
#include "locks.h"
#include "syscalls.h"

#include "hardware/structs/systick.h"
#include "hardware/timer.h"

struct next {
	struct thread_handle* th;
	struct next *next;
};

// priority queue
static struct {
	struct spinlock lock;
	struct next *first;
	int size;
	uint32_t id;
} sched;


void add_on_node(struct next *node, void *ptr)
{
	struct next *tmp = malloc(sizeof(struct next));
	tmp->th = (struct thread_handle *)ptr;
	tmp->next = node->next;
	node->next = tmp;
}

void push_th(struct thread_handle *th)
{
	acquire_lock(&sched.lock);
	
	struct next *iter = sched.first;
	if (iter == NULL) {
		sched.first = malloc(sizeof(struct next));
		sched.first->th = th;
		sched.first->next = NULL;
		sched.size = 1;
		release_lock(&sched.lock);
		return;
	}

	while (iter != NULL)
	{
		// add at the end
		if (iter->next == NULL) {
			add_on_node(iter, th);
			sched.size++;
			release_lock(&sched.lock);
			return;
		}

		// higher priority than current node
		// swap thread handles and add next
		struct thread_handle *th_iter = iter->th;
		if (th->priority > th_iter->priority) {
			iter->th = th;
			add_on_node(iter, th_iter);
			sched.size++;
			release_lock(&sched.lock);
			return;
		}

		iter = iter->next;
	}

	release_lock(&sched.lock);
	panic("failed to push thread on priority queue");
}

struct thread_handle *pop_th(void)
{
	acquire_lock(&sched.lock);
	if (!sched.first) {
		release_lock(&sched.lock);
		return NULL;
	}

	struct next *tmp = sched.first;
	struct thread_handle *th = tmp->th;

	sched.first = sched.first->next;
	free(tmp);
	sched.size--;

	release_lock(&sched.lock);

	return th;
}


static struct thread_handle *idle;

static void idle_thread(__unused void *arg)
{
	while (1)
		__WFI();
}

void sched_init(void)
{
	init_lock(&sched.lock, "scheduler");
	struct thread_attr atr = {
		.name = "idle",
		.priority = prio_low,
		.stack_size = PAGE_SIZE
	};

	thread_create(idle_thread, NULL, &atr);
	idle = sched.first->th;

	// clear scheduler
	free(sched.first); // sched.first = sched.last = NULL;
}

// ask scheduler for next thread to run!
// is thread safe
static struct thread_handle *get_runnable(void)
{
	struct thread_handle *th = pop_th();
	if (!th)
		th = idle;

	return th;	
}


//TODO:
static void preempt(struct thread_handle *next, struct thread_handle *running)
{
	if( next->priority < running->priority)
		next->priority = running->priority;
}


uint32_t sched_add_thread(struct thread_handle* th)
{
	struct next *tmp = malloc(sizeof(struct next));
	tmp->th = th;
	uint32_t id;

	id = sched.id++;
	push_th(th);

	return id;
}


#define SYSTEM_CLOCK 	125000000UL
#define SYS_TICK		SYSTEM_CLOCK/1000 -1

#ifdef USE_PRIV_THREADS
	#define THREAD_STATUS		PRIV_TH
#else
	#define THREAD_STATUS		UNPRIV_TH
#endif

void start_sched(void)
{
	if (sched_runtime())
		return;
	
	NVIC_SetPriority(PendSV_IRQn, 0xff);
	NVIC_SetPriority(SysTick_IRQn, 0xff); // due to irq overlaping

	systick_hw->csr = 0;
	systick_hw->rvr = SYS_TICK;
	systick_hw->cvr = 0;
	systick_hw->csr = 0b0111;

	struct thread_handle *run = get_runnable();
	if (!run)
		panic("no runnable threads");
	
	__set_PSP(run->stack_ptr);

	__set_CONTROL(THREAD_STATUS);
	__ISB();

	// consume initial stack frame
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
} sched_status;


uint32_t systick_counter = 0;


void* pre_switch(struct thread_handle *running)
{
	sched_status.run = running;
	sched_status.next = get_runnable();

	return &sched_status; // 0x2000f20c
}

// check if sched started running 
int sched_runtime(void)
{
	return 0;
}

extern void context_switch(void *ptr);

static void scheduler(struct thread_handle *running, struct thread_handle *next)
{
	// don't schedule threads
	systick_counter = 0;

	if (unlikely(running->state == Running))
		panic("scheduler called on - thread running");

	if (unlikely(running != mythread()))
		panic("thread mismatch");

	if (!next) // next is NULL get a runnable thread
		next = get_runnable();

	sched_status.run = running;
	sched_status.next = next;

	// int prio = preempt(next, running);

	// printk("bye bye %s\n", running->name);
	sys_switch(&sched_status);
	//context_switch(&sched_status);
	// printk("wake up %s\n", running->name);

	// next->priority = prio;
}

//TODO: order by priority
static void add_blocked(struct mutex *mtx, struct thread_handle *th)
{
	struct next *lst = malloc(sizeof(struct next));
	lst->th = th;
	lst->next = mtx->blocked;
	mtx->blocked = lst;
}

#include "pico/time.h"

static int64_t set_ready(alarm_id_t id, void *data)
{
	struct thread_handle *th = (struct thread_handle *)data;
	th->state = Ready;
	return 0;
}

void yield(struct thread_handle *th, uint32_t ms)
{
	th->state = Waiting;
	add_alarm_in_ms(ms, set_ready, th, true);
	scheduler(th, NULL);
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

// get thread sleeping on mutex
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

	sched_status.run = mythread();
	sched_status.next = get_runnable();	

	release_lock(&mtx->sp_lk);

	if (holding(&mtx->sp_lk))
		printk("mutex sp lck %s is held\n", mtx->sp_lk.name);

	sys_switch(&sched_status);

	acquire_lock(&mtx->sp_lk);
}

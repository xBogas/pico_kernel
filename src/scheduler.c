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


static struct thread_handle *idle_core0;
static struct thread_handle *idle_core1;

static void idle_thread(void *arg)
{
	while (1)
		__WFI();
}

void sched_init(void)
{
	init_lock(&sched.lock, "scheduler");
	struct thread_attr atr = {
		.name = "idle_core0",
		.priority = prio_idle,
		.stack_size = PAGE_SIZE
	};

	// create idle thread for core 0
	thread_create(idle_thread, &idle_core0, &atr);
	idle_core0 = pop_th();

	// create idle thread for core 1
	atr.name = "idle_core1";
	thread_create(idle_thread, &idle_core1, &atr);
	idle_core1 = pop_th();
}


static struct thread_handle *get_idle(void)
{
	return cpu_id() == 0 ? idle_core0 : idle_core1;
}


// ask scheduler for next thread to run with higher priority than prio
// is thread safe
static struct thread_handle *get_runnable(uint16_t prio)
{
	struct thread_handle *th;

	acquire_lock(&sched.lock);
	struct next *tmp = sched.first;
	while (tmp) {
		th = tmp->th;
		if ((th->state == Ready) && (th->priority > prio)) {
			th->state = Running;
			release_lock(&sched.lock);
			return th;
		}

		tmp = tmp->next;
	}

	release_lock(&sched.lock);
	return cpu_id() == 0 ? idle_core0 : idle_core1;
}


// TODO:
// static void preempt(struct thread_handle *next, struct thread_handle *running)
// {
// 	if( next->priority < running->priority)
// 		next->priority = running->priority;
// }


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

#define trig_pendsv()		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk

void start_sched(void)
{
	if (sched_runtime())
		return;

	NVIC_SetPriority(PendSV_IRQn, 0xff);
	NVIC_SetPriority(SVCall_IRQn, 0xff);
	NVIC_SetPriority(SysTick_IRQn, 0x00); 		// due to irq overlaping
	NVIC_SetPriority(TIMER_IRQ_3_IRQn, 0x0f);

	systick_hw->csr = 0;
	systick_hw->rvr = SYS_TICK;
	systick_hw->cvr = 0;
	systick_hw->csr = 0b0111;

	__ISB();
	struct thread_handle *run = get_runnable(prio_idle);
	if (!run)
		panic("no runnable threads");

	sys_sched(run->stack_ptr);
}

struct thread_switch {
	struct thread_handle *run;
	struct thread_handle *next;
};

struct thread_switch sched_core0;
struct thread_switch sched_core1;


void* pre_switch(void)
{
	struct thread_handle *running = mythread();
	struct thread_handle *next = get_runnable(running->priority);

	if (cpu_id() != 0) {

		if (next == idle_core1)
			return NULL;

		if (next == running)
			return NULL;

		// printk("core1 isr %s -> %s\n", running->name, next->name);

		sched_core1.run = running;
		sched_core1.next = next;

		running->state = Ready;
		return &sched_core1;
	}

	if (next == idle_core0)
		return NULL;

	if (next == running)
		return NULL;

	// printk("core0 isr %s -> %s\n", running->name, next->name);

	sched_core0.run = running;
	sched_core0.next = next;

	running->state = Ready;
	return &sched_core0;
}

void isr_systick(void)
{
	void *ptr = pre_switch();
	if (ptr)
		trig_pendsv();
}


// check if sched started running 
int sched_runtime(void)
{
	return 0;
}

extern void context_switch(void *ptr);

static void scheduler(struct thread_handle *running, struct thread_handle *next)
{
	if (unlikely(running->state == Running))
		return;

	if (unlikely(running != mythread()))
		panic("thread mismatch");

	if (!next) // next is NULL get a runnable thread
		next = get_runnable(prio_idle);

	if (running == next)
		return;

	if (cpu_id() != 0)
	{
		sched_core1.run = running;
		sched_core1.next = next;

		// printk("core1 scheduling %s -> %s\n", running->name, next->name);
		sys_switch(&sched_core1);
		return;
	}

	sched_core0.run = running;
	sched_core0.next = next;
	// printk("core0 scheduling %s -> %s\n", running->name, next->name);
	sys_switch(&sched_core0);
}

// add thread to mutex blocked list
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
	struct thread_handle *owner = mtx->owner;
	struct thread_handle *next = owner;

	// if owner is running now reschedule another thread
	if (owner->state == Running)
		next = NULL;

	release_lock(lk);

	scheduler(th, next); // must reschedule mutex owner

	th->state = Running;
	th->obj = 0;

	if (holding(lk))
		printk("mutex sp lck %s is held\n", lk->name);

	acquire_lock(&mtx->sp_lk);
}

// wake up threads sleeping on mutex
// return if any thread with higher priority was woken up
static struct thread_handle *wake_blocked(struct mutex *mtx, uint16_t prio)
{
	struct next *lst = NULL;
	struct thread_handle *th = NULL;
	struct thread_handle *max = NULL;
	while (mtx->blocked)
	{
		lst = (struct next *)mtx->blocked;
		th = lst->th;

		if(th->priority > prio) {
			max = lst->th;
			prio = max->priority;
		} else 
			th->state = Ready;

		mtx->blocked = lst->next;
		free(lst);
	}

	return max;
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

	struct thread_handle *run = mythread();
	struct thread_handle *th = wake_blocked(mtx, run->priority);
	if (!th)
		return;

	if (th->state == Running)
		return;

	run->state = Blocked;
	run->obj = mtx;
	add_blocked(mtx, run);

	th->state = Running;
	release_lock(&mtx->sp_lk);

	printk("waking up %s\n", th->name);
	scheduler(run, th);

	acquire_lock(&mtx->sp_lk);
}

#include "kernel.h"
#include "terminal.h"
#include "scheduler.h"
#include "memory.h"
#include "thread.h"

static struct mutex mtx;

void task2(__unused void *data)
{
	int i = 0;
	while (1) {
		acquire_mutex(&mtx);
		printk("hi! foo [%d]\n", i);
		release_mutex(&mtx);

		wait(750);
		i++;
	}
}

void task3(__unused void *data)
{
	int i = 0;
	while (1) {
		acquire_mutex(&mtx);
		printk("hi! bar [%d]\n", i);
		release_mutex(&mtx);

		wait(1000);
		i++;
	}
}

static int ready = 0;
int main(void){
	
	if (cpu_id() == 0){
		printk("core 0 entered main\n");
		k_mem_init();
		sched_init();
		init_mutex(&mtx, "mtx_test");

		struct thread_attr atr = {
			.name = "task1",
			.priority = prio_def,
			.stack_size = 0x400
		};
		atr.priority = prio_high;
		thread_create(&task2, NULL, &atr);

		atr.name = "task2";
		atr.priority = prio_max;
		thread_create(&task3, NULL, &atr);

		ready = 1;
	}
	else {
		printk("core 1 running ...\n");
		while (1)
			bs_wait(1);
	}

	start_sched();
	while (1)
		;

	return 0;
}
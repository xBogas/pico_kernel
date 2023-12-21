#include "terminal.h"
#include "scheduler.h"
#include "memory.h"
#include "thread.h"

static struct mutex mtx;

static int i = 0;

void task1(__unused void *data)
{
	while (1) {
		acquire_mutex(&mtx);
		// printk("hi! bar [%d]\n", i);
		// bs_wait(10);
		release_mutex(&mtx);

		i++;
	}
}

void task2(__unused void *data)
{
	while (1) {
		acquire_mutex(&mtx);
		// printk("hi! foo [%d]\n", i);
		// bs_wait(10);
		release_mutex(&mtx);

		i++;
	}
}

void task3(__unused void *data)
{
	int i = 0;
	while (1) {
		acquire_mutex(&mtx);
		// printk("hi! foobar [%d]\n", i);
		// bs_wait(10);
		release_mutex(&mtx);

		i++;
	}
}


int main(void){
	if (cpu_id() == 0){
		printk("core 0 entered main\n");
		k_mem_init();
		sched_init();

		init_mutex(&mtx, "mtx_test");
	
		struct thread_attr atr = {
			.name = "task1",
			.priority = 1,
			.stack_size = 0x400
		};

		thread_create(&task1, NULL, &atr);

		atr.name = "task2";
		thread_create(&task2, NULL, &atr);

		atr.name = "task3";
		thread_create(&task3, NULL, &atr);
	}
	else {
		printk("core 1 running ...\n");
		while (1)
			;
	}

	start_sched();
	while (1)
		;

	return 0;
}
#include "kernel.h"
#include "terminal.h"
#include "scheduler.h"
#include "memory.h"
#include "thread.h"

static struct mutex mtx;

struct task_param
{
	const char *str;
	uint32_t timer;
};


void task(void *data)
{
	struct task_param *param = (struct task_param *)data;
	const char *str = param->str;
	int i = 0;
	while (1) {
		acquire_mutex(&mtx);
		printk("hi! %s [%d] from %d\n", str, i++, cpu_id());
		release_mutex(&mtx);

		wait(param->timer);
	}
}


void add_template_task(const char* name, uint16_t prio, uint32_t timer)
{
	struct thread_attr atr = {
		.name = name,
		.priority = prio,
		.stack_size = 0x400
	};

	struct task_param *param = malloc(sizeof(struct task_param));

	param->str = name;
	param->timer = timer;

	thread_create(&task, param, &atr);
}

static int ready = 0;
int main(void){
	
	if (cpu_id() == 0){
		printk("core 0 entered main\n");
		k_mem_init();
		sched_init();
		init_mutex(&mtx, "mtx_test");

		add_template_task("foo", 	prio_def, 100);
		add_template_task("bar", 	prio_low, 250);
		// add_template_task("foobar", prio_max, 500);

		ready = 1;
		__SEV();
	}
	else {
		printk("core 1 running ...\n");
		while (ready != 1)
			__WFE();
		
		busy_wait_at_least_cycles(100);
	}

	start_sched();
	while (1)
		;

	return 0;
}
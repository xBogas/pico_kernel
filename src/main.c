#include "kernel.h"
#include "terminal.h"
#include "scheduler.h"
#include "memory.h"
#include "thread.h"
#include "wifi.h"

static struct mutex mtx;

void task1(__unused void *data)
{
	uint32_t start = time_us_32();
	bool state = true;

	while (1) {
		if (time_us_32() - start > 1000000) {
			start = time_us_32();
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
			state = !state;
		}
		wait(50);
	}
}

void scan(__unused void *data)
{
	uint32_t start = time_us_32();

	while (1) {
		if (time_us_32() - start > 3000000) {
			start = time_us_32();
			acquire_mutex(&mtx);
			wifi_scan();
			release_mutex(&mtx);
		}

		wait(500);
	}
}



void task2(__unused void *data)
{
	int i = 0;
	while (1) {
		acquire_mutex(&mtx);
		printk("hi! foo [%d]\n", i);
		release_mutex(&mtx);

		wait(500);
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

		wait(500);
		i++;
	}
}


int main(void){
	if (cpu_id() == 0){
		printk("core 0 entered main\n");
		k_mem_init();
		sched_init();
		wifi_init();
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

		atr.name = "wifi scan";
		thread_create(&scan, NULL, &atr);
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
#include "terminal.h"
#include "scheduler.h"
#include "memory.h"
#include "thread.h"
#include "hardware/timer.h"

void task1(__unused void *data)
{
	int i = 0;
	while (1) {
		printk("[%d]	hi! bar\n", i);
		busy_wait_ms(500);
		i++;
	}
}

void task2(__unused void *data)
{
	int i = 0x7FFFFFFF;
	while (1) {
		printk("[%d]	hi! foo\n", i);
		busy_wait_ms(100);
		i--;
	}
}

int main(void){
	if (get_core_num() == 0){
		printk("core 0 entered main\n");
		k_mem_init();

		struct thread_attr atr_1 = {
			.name = "task1",
			.priority = 1,
			.stack_size = 0x400
		};

		struct thread_attr atr_2 = {
			.name = "task2", // name can go out of scope
			.priority = 1,
			.stack_size = 0x400
		};

		thread_create(&task1, NULL, &atr_1);
		thread_create(&task2, NULL, &atr_2);

	} //pc at 0x10000522 <main+42>
	else {
		printk("core 1 running ...\n");
		while (1)
			;//usb_task();
	}

	start_sched();
	while (1)
		;

	return 0;
}
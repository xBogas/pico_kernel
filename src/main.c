#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "scheduler.h"
#include "init.h"
#include "hardware/structs/scb.h"

void thread(void* data) {
	(void)data;
}

int main(void) {
	if (cpuid() == 0){
		printf("core 0 entered main\n");

		struct thread_attr atr_1  = {
			.name = "thread 1",
			.priority = 1
		};

		int pid = thread_create(thread, NULL, &atr_1);
		printf("new pid %d\n", pid);

		struct thread_attr atr_2 = {
			.name = "thread 2",
			.priority = 1
		};

		pid = thread_create(thread, NULL, &atr_2);
		printf("new pid %d\n", pid);

		//start_sched();
		sleep_ms(5000);
	}
	else {
		printf("core 1 entered main\n");
		while (1){
			sleep_ms(3000);
			printf("core 1 running\n");
		}
	}

	return 0;
}
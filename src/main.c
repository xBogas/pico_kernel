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
	// enable entry point to start console and launch multicore ?
	// this entry point can be with attributes "__attribute__((constructor))" or change bootloader
	// config console + printf -> enable debug
	// init memory allocation
	// init memory mapping ? may be required by console/printf
	// 
	if (cpuid() == 0){
        // this should not be called here
        kernel_entry();
		printf("core 0 entered main\n");

		sleep_ms(500);

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
        // ready = 1;
		while (1)
			;
	}
	else {
		printf("core 1 entered main\n");
        // while (ready == 0)
        //     ;
		while (1)
			;
	}

	return 0;
}
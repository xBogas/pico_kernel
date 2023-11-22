#include "stdio.h"
#include "memory.h"
#include "init.h"

int main(void)
{
	if (cpuid() == 0){
		printf("core 0 entered main\n");
		k_mem_init();
		
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
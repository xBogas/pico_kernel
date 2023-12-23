#ifndef W_KERNEL_WIFI_H
#define W_KERNEL_WIFI_H

#include "kernel.h"
#include "terminal.h"

#include "pico/cyw43_arch.h"

static void wifi_init(void)
{
	if(cyw43_arch_init()){
		panic("wifi init failed\n");
	}
}

#endif
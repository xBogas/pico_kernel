#ifndef W_KERNEL_INIT_H
#define W_KERNEL_INIT_H

#include "hardware/structs/sio.h"

static inline int cpuid() {
	return sio_hw->cpuid;
}

void kernel_entry(void);

#endif
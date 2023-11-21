#ifndef W_KERNEL_INIT_H
#define W_KERNEL_INIT_H

#include <stdint.h>
#include "pico/stdio.h"
#include "pico/multicore.h"
#include "hardware/resets.h"
#include "hardware/clocks.h"

// hardware registers
#include "hardware/structs/sio.h"
#include "hardware/structs/scb.h"
#include "hardware/regs/resets.h"

static inline int cpuid() {
	return sio_hw->cpuid;
}

void kernel_entry(void);

#endif
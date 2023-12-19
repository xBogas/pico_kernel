#ifndef W_KERNEL_H_INCLUDED_
#define W_KERNEL_H_INCLUDED_

#define likely(x)       __builtin_expect(x, 1)
#define unlikely(x)     __builtin_expect(x, 0)

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "pico/platform.h"
#include "RP2040.h"

void __attribute__((noreturn)) panic(const char *fmt, ...);

void __attribute__((noreturn)) exit(int status);

static inline uint32_t is_privileged(void)
{
	return (__get_current_exception()) || !(__get_CONTROL() & 1);
}

#define cpu_id()		get_core_num()

#endif
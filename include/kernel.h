#ifndef W_KERNEL_H_INCLUDED_
#define W_KERNEL_H_INCLUDED_

#define likely(x)       __builtin_expect(x, 1)
#define unlikely(x)     __builtin_expect(x, 0)

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "pico/platform.h"

void __attribute__((noreturn)) panic(const char *fmt, ...);

void __attribute__((noreturn)) exit(int status);

#define cpu_id()		get_core_num()

#endif
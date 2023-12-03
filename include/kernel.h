#ifndef W_KERNEL_H_INCLUDED_
#define W_KERNEL_H_INCLUDED_

#define likely(x)       __builtin_expect(x, 1)
#define unlikely(x)     __builtin_expect(x, 0)

void __attribute__((noreturn)) panic(const char *fmt, ...);

#endif
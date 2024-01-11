#ifndef W_KERNEL_TERMINAL_H_INCLUDED
#define W_KERNEL_TERMINAL_H_INCLUDED

#include <stdarg.h>

void panic_terminal();

void term_init(void);
void printk(const char *str, ...);
//void vprintk(const char *fmt, va_list ap, int ppanic);

void force_printk(const char *str, ...);

#if KERNEL_CONSOLE_USB
int term_connected(void);
void usb_task(void);
#endif

#endif
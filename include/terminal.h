#ifndef W_KERNEL_TERMINAL_H_INCLUDED
#define W_KERNEL_TERMINAL_H_INCLUDED

int term_connected(void);
void term_init(void);
void term_flush(void);
void printk(const char *str);
void usb_task(void);
#endif
#include "kernel.h"
#include "pico/types.h"
#include "locks.h"
#include "terminal.h"

#if !defined(KERNEL_CONSOLE_UART) && !defined(KERNEL_CONSOLE_USB)
	#define KERNEL_CONSOLE_UART 1
	#define KERNEL_CONSOLE_USB 0
#endif

#if KERNEL_CONSOLE_UART

#include "hardware/uart.h"
#include "hardware/gpio.h"


#define c_write(c) uart_putc_raw(uart0, c)

static void init_uart(void)
{
	uart_init(uart0, 115200);
	gpio_set_function(0, GPIO_FUNC_UART);
	gpio_set_function(1, GPIO_FUNC_UART);
}

#elif // KERNEL_CONSOLE_USB

#include "tusb.h"
#include "pico/stdio_usb.h"
#include "hardware/irq.h"
#include "RP2040.h"

#define c_write(c) tud_cdc_write(&c, 1)

int term_connected(void)
{
	return tud_cdc_connected();
}

static struct spinlock _usb_task;

static void init_usb(void)
{
	//tusb_init();
	stdio_usb_init();
	init_lock(&_usb_task, "usb_task_lock");
}

void usb_task(void)
{
	acquire_lock(&_usb_task);
	tud_task();
	release_lock(&_usb_task);
}

void 
#endif

static struct spinlock print;
static volatile int panicked = 0; 

void panic_terminal()
{
	panicked = 1;
}

void term_init(void)
{
	init_lock(&print, "print_lock");

#if KERNEL_CONSOLE_UART
	init_uart();
#endif

#if KERNEL_CONSOLE_USB
	init_usb();
	while (!term_connected())
		;
#endif
}

void vprintk(const char *fmt, va_list ap, int ppanic);
static void print_str(char *str, int ppanic);
static void print_hex(uint32_t i, int ppanic);
static void print_int(int i, int ppanic);
static void c_put(char c, int ppanic);

void panic_unsupported(void)
{
	uint32_t lr;
	__asm volatile("mov %0, lr" : "=r"(lr));
	panic("unsupported %x\n", lr);
}

void panic(const char *format, ...)
{
	// force unlock print lock
	release_lock(&print);
	acquire_lock(&print);
    printk("PANIC!: ");
    va_list args;
    va_start(args, format);
    vprintk(format, args, 0);
    va_end(args);
	printk("\n");
	panicked = 1;
	exit(1);
}

void printk(const char *fmt, ...)
{
	if (!fmt)
		return;

	va_list ap;
	va_start(ap, fmt);
	vprintk(fmt, ap, panicked);
	va_end(ap);

#if KERNEL_CONSOLE_USB
	if((__get_current_exception() != 0) && NVIC_GetPendingIRQ(USBCTRL_IRQ)) {
		irq_handler_t irq = irq_get_vtable_handler(USBCTRL_IRQ);
		irq();
		usb_task();
	}
#endif
}


void vprintk(const char *fmt, va_list ap, int ppanic)
{
	if (!__get_current_exception())
		acquire_lock(&print);

	while (*fmt != '\0') {
		if (*fmt != '%') {
			c_put(*fmt++, ppanic);
			continue;
		}

		switch (*(++fmt)) {
		case 'd':
			print_int(va_arg(ap, int), ppanic);
			break;
		case 'x':
			print_hex(va_arg(ap, int), ppanic);
			break;
		case 's':
			print_str(va_arg(ap, char *), ppanic);
			break;
		case 'p':
			print_hex((uint32_t)va_arg(ap, uint32_t *), ppanic);
			break;
		case 'c':
			c_put(va_arg(ap, int), ppanic);
			break;
		default: // not implemented
			c_put('%', ppanic);
			c_put(*fmt, ppanic);
			break;
		}
		fmt++;
	}

	if (!__get_current_exception())
		release_lock(&print);
}

void force_printk(const char *str, ...)
{
	va_list ap;
	va_start(ap, str);
	vprintk(str, ap, 0);
	va_end(ap);
}

static void c_put(char c, int ppanic)
{
	if (unlikely(ppanic))
		while (1);
	
	if (unlikely(c == '\n')){
		c_write('\r');
		c_write('\n');
	}
	else
		c_write(c);
}



static void print_int(int i, int ppanic)
{
	char buf[16];
	int len = 0;
	int neg = 0;

	if (i == 0) {
		c_put('0', ppanic);
		return;
	}

	if (i < 0) {
		neg = 1;
		i = -i;
	}

	while (i > 0) {
		buf[len++] = '0' + (i % 10);
		i /= 10;
	}

	if (neg)
		c_put('-', ppanic);

	while (len > 0)
		c_put(buf[--len], ppanic);
}

static void print_hex(uint32_t i, int ppanic)
{
	c_put('0', ppanic);
	c_put('x', ppanic);

	char buf[16];
	int len = 0;

	if (i == 0) {
		c_put('0', ppanic);
		return;
	}

	while (i > 0) {
		int digit = i % 16;
		if (digit < 10)
			buf[len++] = '0' + digit;
		else
			buf[len++] = 'a' + digit - 10;
		i /= 16;
	}

	while (len > 0)
		c_put(buf[--len], ppanic);
}

static void print_str(char *str, int ppanic)
{
	if (!str)
		str = "(null)";

	while (*str != '\0')
		c_put(*str++, ppanic);
}

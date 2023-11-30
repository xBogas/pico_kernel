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


#define likely(x)       __builtin_expect(x, 1)
#define unlikely(x)     __builtin_expect(x, 0)

static struct spinlock print;

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

void vprintk(const char *fmt, va_list ap);
static void print_ptr(uint32_t *ptr);
static void print_str(char *str);
static void print_hex(uint32_t i);
static void print_int(int i);
static void c_put(char c);


void printk(const char *fmt, ...)
{
	if (!fmt)
		return;

	va_list ap;
	va_start(ap, fmt);
	vprintk(fmt, ap);
	va_end(ap);

#if KERNEL_CONSOLE_USB
	if((__get_current_exception() != 0) && NVIC_GetPendingIRQ(USBCTRL_IRQ)) {
		irq_handler_t irq = irq_get_vtable_handler(USBCTRL_IRQ);
		irq();
		usb_task();
	}
#endif
}


void vprintk(const char *fmt, va_list ap)
{
	acquire_lock(&print);
	while (*fmt != '\0') {
		if (*fmt != '%') {
			c_put(*fmt++);
			continue;
		}

		switch (*(++fmt)) {
		case 'd':
			print_int(va_arg(ap, int));
			break;
		case 'x':
			print_hex(va_arg(ap, int));
			break;
		case 's':
			print_str(va_arg(ap, char *));
			break;
		case 'p':
			print_ptr(va_arg(ap, uint32_t *));
			break;
		case 'c':
			c_put(va_arg(ap, int));
			break;
		default: // not implemented
			c_put('%');
			c_put(*fmt);
			break;
		}
		fmt++;
	}
	release_lock(&print);
}

static void c_put(char c)
{
	if (unlikely(c == '\n')){
		c_write('\r');
		c_write('\n');
	}
	else
		c_write(c);
}



static void print_int(int i)
{
	char buf[16];
	int len = 0;
	int neg = 0;

	if (i == 0) {
		c_put('0');
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
		c_put('-');

	while (len > 0)
		c_put(buf[--len]);
}

static void print_hex(uint32_t i)
{
	char buf[16];
	int len = 0;

	if (i == 0) {
		c_put('0');
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
		c_put(buf[--len]);
}

static void print_str(char *str)
{
	if (!str)
		str = "(null)";

	while (*str != '\0')
		c_put(*str++);
}

static void print_ptr(uint32_t *ptr)
{
	c_put('0');
	c_put('x');
	print_hex((uint32_t)ptr);
}
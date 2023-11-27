#include "RP2040.h"
#include <stdlib.h>
#include "terminal.h"
#include "pico/printf.h"

void isr_invalid(void)
{
    printk("NVIC invalid called");
	exit(1);
}

void isr_nmi(void)
{
    printk("NVIC nmi called");
	exit(1);
}

void isr_hardfault(void)
{
    printk("NVIC hardfault called\n");
    printk("test lr mv\n");

    uint32_t lr_status = 0;
    asm volatile(
        "mov %0, lr" : "=r" (lr_status)
    );

    char buf[32];
    sprintf(buf, "lr: %x\n", lr_status);
    printk(buf);

    CONTROL_Type control_status;
    control_status.w = __get_CONTROL();
    sprintf(buf, "control: %d\n", control_status.b.SPSEL);
    printk(buf);
    //uint32_t ipsr_status = __get_IPSR();
    //sprintf(buf, "ipsr: %x\n", ipsr_status);
    //printk(buf);
    printk("12345678\n"); 
    exit(1);
}

void isr_svcall(void)
{
    printk("NVIC svcall called");
}

void isr_pendsv(void)
{
    printk("NVIC pendsv called");
}

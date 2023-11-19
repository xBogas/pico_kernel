#include "init.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/structs/scb.h"


extern int main();

void kernel_entry(void) {
	stdio_init_all();
	while (!stdio_usb_connected())
		;
	sleep_ms(500);
	multicore_launch_core1(main);
	printf("exit kernel_entry\n");
}

#include "hardware/regs/resets.h"

uint32_t __attribute__((section(".ram_vector_table"))) ram_vector_table[48];

// entry point
void runtime_init(void) {

	// Reset all peripherals to put system into a known state,
    // - except for QSPI pads and the XIP IO bank, as this is fatal if running from flash
    // - and the PLLs, as this is fatal if clock muxing has not been reset on this boot
    // - and USB, syscfg, as this disturbs USB-to-SWD on core 1
	reset_block(~(
            RESETS_RESET_IO_QSPI_BITS |
            RESETS_RESET_PADS_QSPI_BITS |
            RESETS_RESET_PLL_USB_BITS |
            RESETS_RESET_USBCTRL_BITS |
            RESETS_RESET_SYSCFG_BITS |
            RESETS_RESET_PLL_SYS_BITS
    ));

    // Remove reset from peripherals which are clocked only by clk_sys and
    // clk_ref. Other peripherals stay in reset until we've configured clocks.
    unreset_block_wait(RESETS_RESET_BITS & ~(
            RESETS_RESET_ADC_BITS |
            RESETS_RESET_RTC_BITS |
            RESETS_RESET_SPI0_BITS |
            RESETS_RESET_SPI1_BITS |
            RESETS_RESET_UART0_BITS |
            RESETS_RESET_UART1_BITS |
            RESETS_RESET_USBCTRL_BITS
    ));

	// call preinit functions
	extern void (*__preinit_array_start)(void);
    extern void (*__preinit_array_end)(void);
    for (void (**p)(void) = &__preinit_array_start; p < &__preinit_array_end; ++p) {
        (*p)();
    }

	clocks_init();
    unreset_block_wait(RESETS_RESET_BITS);

    __builtin_memcpy(ram_vector_table, (uint32_t *) scb_hw->vtor, sizeof(ram_vector_table));
    scb_hw->vtor = (uintptr_t) ram_vector_table;

    spin_locks_reset();
    irq_init_priorities();
    alarm_pool_init_default();

    // Start and end points of the constructor list,
    // defined by the linker script.
    extern void (*__init_array_start)(void);
    extern void (*__init_array_end)(void);

    // Call each function in the list.
    // We have to take the address of the symbols, as __init_array_start *is*
    // the first function pointer, not the address of it.
    for (void (**p)(void) = &__init_array_start; p < &__init_array_end; ++p) {
        (*p)();
    }

    kernel_entry();
}


// C standard library functions
#include <stdarg.h>

#include "pico/bootrom.h"

void __attribute__((noreturn)) _exit(int status) {
	// set LED on
	reset_usb_boot(0, 0);
}

void panic(const char *format, ...) {
	//TODO: print to console
	_exit(1);
}

void hard_assertion_failure(void) {
    panic("Hard assert failed");
}


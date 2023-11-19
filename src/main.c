#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "scheduler.h"
#include "init.h"
#include "hardware/structs/scb.h"
#include "hardware/structs/padsbank0.h"

void thread(void* data) {
	(void)data;
}

int arr_size = 0;
int arr1_size = 0;

int runtime_init(void) {

	// runtime_init_default();
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

    // pre-init runs really early since we need it even for memcpy and divide!
    // (basically anything in aeabi that uses bootrom)

    // Start and end points of the constructor list,
    // defined by the linker script.
    extern void (*__preinit_array_start)(void);
    extern void (*__preinit_array_end)(void);

    // Call each function in the list.
    // We have to take the address of the symbols, as __preinit_array_start *is*
    // the first function pointer, not the address of it.
    for (void (**p)(void) = &__preinit_array_start; p < &__preinit_array_end; ++p) {
        (*p)();
		arr_size++;
    }

    // After calling preinit we have enough runtime to do the exciting maths
    // in clocks_init
    clocks_init();

    // Peripheral clocks should now all be running
    unreset_block_wait(RESETS_RESET_BITS);

    // after resetting BANK0 we should disable IE on 26-29
    padsbank0_hw_t *padsbank0_hw_clear = (padsbank0_hw_t *)hw_clear_alias_untyped(padsbank0_hw);
    padsbank0_hw_clear->io[26] = padsbank0_hw_clear->io[27] =
            padsbank0_hw_clear->io[28] = padsbank0_hw_clear->io[29] = PADS_BANK0_GPIO0_IE_BITS;

    // this is an array of either mutex_t or recursive_mutex_t (i.e. not necessarily the same size)
    // however each starts with a lock_core_t, and the spin_lock is initialized to address 1 for a recursive
    // spinlock and 0 for a regular one.

    static_assert(!(sizeof(mutex_t)&3), "");
    static_assert(!(sizeof(recursive_mutex_t)&3), "");
    static_assert(!offsetof(mutex_t, core), "");
    static_assert(!offsetof(recursive_mutex_t, core), "");
    extern lock_core_t __mutex_array_start;
    extern lock_core_t __mutex_array_end;

    for (lock_core_t *l = &__mutex_array_start; l < &__mutex_array_end; ) {
        if (l->spin_lock) {
            assert(1 == (uintptr_t)l->spin_lock); // indicator for a recursive mutex
            recursive_mutex_t *rm = (recursive_mutex_t *)l;
            recursive_mutex_init(rm);
            l = &rm[1].core; // next
        } else {
            mutex_t *m = (mutex_t *)l;
            mutex_init(m);
            l = &m[1].core; // next
        }
    }

    __builtin_memcpy(ram_vector_table, (uint32_t *) scb_hw->vtor, sizeof(ram_vector_table));
    scb_hw->vtor = (uintptr_t) ram_vector_table;

#ifndef NDEBUG
#warning "Exception handlers are not set up yet"
    if (__get_current_exception()) {
        // crap; started in exception handler
        __breakpoint();
    }
#endif

    // Start and end points of the constructor list,
    // defined by the linker script.
    extern void (*__init_array_start)(void);
    extern void (*__init_array_end)(void);

    // Call each function in the list.
    // We have to take the address of the symbols, as __init_array_start *is*
    // the first function pointer, not the address of it.
    for (void (**p)(void) = &__init_array_start; p < &__init_array_end; ++p) {
        (*p)();
		arr1_size++;
    }

	kernel_entry();
	return 0;
}

int main(void) {
	// enable entry point to start console and launch multicore ?
	// this entry point can be with attributes "__attribute__((constructor))" or change bootloader
	// config console + printf -> enable debug
	// init memory allocation
	// init memory mapping ? may be required by console/printf
	// 
	printf("arr_size %d\n", arr_size);
	printf("arr_size 1 %d\n", arr1_size);
	if (cpuid() == 0){
		printf("core 0 entered main\n");

		sleep_ms(500);

		struct thread_attr atr_1  = {
			.name = "thread 1",
			.priority = 1
		};

		int pid = thread_create(thread, NULL, &atr_1);
		printf("new pid %d\n", pid);

		struct thread_attr atr_2 = {
			.name = "thread 2",
			.priority = 1
		};

		pid = thread_create(thread, NULL, &atr_2);
		printf("new pid %d\n", pid);

		//start_sched();
		while (1)
			;
	}
	else {
		printf("core 1 entered main\n");
		while (1)
			;
	}

	return 0;
}
#include "terminal.h"
#include "sio_fifo.h"

#include "pico/bootrom.h"

// hardware registers
#include "hardware/resets.h"
#include "hardware/clocks.h"
#include "hardware/irq.h"
#include "hardware/watchdog.h"
#include "hardware/structs/scb.h"
#include "hardware/sync.h"
#include "hardware/structs/padsbank0.h"

#include "pico/mutex.h"
#include "pico/time.h"

extern int main();
void __attribute__((noreturn)) exit(int status);
void launch_core1(void);

uint32_t __attribute__((section(".ram_vector_table"))) ram_vector_table[48];

static void kernel_entry(void)
{
    term_init();
    printk("kernel is booting\n");
	launch_core1();
}

// entry point
void runtime_init(void)
{
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

    // reset pads
    padsbank0_hw_t *padsbank0_hw_clear = (padsbank0_hw_t *)hw_clear_alias_untyped(padsbank0_hw);
    padsbank0_hw_clear->io[26] = padsbank0_hw_clear->io[27] = PADS_BANK0_GPIO0_IE_BITS;
    padsbank0_hw_clear->io[28] = padsbank0_hw_clear->io[29] = PADS_BANK0_GPIO0_IE_BITS;

    __builtin_memcpy(ram_vector_table, (uint32_t *) scb_hw->vtor, sizeof(ram_vector_table));
    scb_hw->vtor = (uintptr_t) ram_vector_table;

    spin_locks_reset();
    irq_init_priorities();
    alarm_pool_init_default();

    // Start and end points of the constructor list,
    // defined by the linker script.
    extern void (*__init_array_start)(void);
    extern void (*__init_array_end)(void);

    // Call constructor functions.
    // For now these are all non essential
    for (void (**p)(void) = &__init_array_start; p < &__init_array_end; ++p) {
        (*p)();
    }

    kernel_entry();
}


void __attribute__((noreturn)) exit(int status)
{
    printk("reseting with status %d\n", status);
    __breakpoint();
    if (status)
        reset_usb_boot(0, 0);

    watchdog_enable(1000, 1);
    while(1);
}


void hard_assertion_failure(void)
{
    panic("Hard assert failed");
}


static void __attribute__((naked)) trampoline(void)
{
    __asm ("pop {r0, pc}");
}


// core 1 entry point
static int entry_point(void)
{
    irq_init_priorities();
    main();
    printk("core 1 main returned");
    return 1;
}

// stack being optimized out for some reason!
static uint32_t __attribute__((section(".stack1"))) core1_stack[PICO_STACK_SIZE / sizeof(uint32_t)];


void launch_core1(void)
{
    extern char __StackOneBottom[];
    extern char __StackOneTop[];
    printk("core 1 stack %x - %x\n", __StackOneBottom, __StackOneTop);

    char *stack = __StackOneBottom;
    uint32_t *stack_ptr = (uint32_t *) (stack + sizeof(core1_stack));

    // memset is replaced by a ROM function
    __builtin_memset(core1_stack, 0, sizeof(core1_stack)); // dont allow to be optimized out

    stack_ptr -= 2;
    stack_ptr[0] = (uint32_t) stack;
    stack_ptr[1] = (uint32_t) entry_point;

    irq_set_enabled(SIO_IRQ_PROC0, false);

    const uint32_t cmds[] = {
        0, 0, 1, (uint32_t) scb_hw->vtor, (uint32_t) stack_ptr, (uint32_t)trampoline
    };

    uint32_t iter = 0;
    do
    {
        uint32_t cmd = cmds[iter];
        if(!cmd) {
            fifo_flush();
            __sev();
        }
        fifo_push(cmd);
        uint32_t rv = fifo_pop();

        if (cmd == rv)
            iter++;
        else
            iter = 0;

    } while (iter < 6);

    irq_set_enabled(SIO_IRQ_PROC0, true);
}
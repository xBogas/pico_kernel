# Boot process

memmap_default.ld will setup the memory layout
_entry_point in crt0.S is called
_reset_handler is called with initializates .data section and clears .bss

core0 will run :

- runtime_init()
- main()
- exit()

core1 will wait in rom function for cmds

preinit functions:

- __aeabi_bits_init - Load ROM functions
- __aeabi_double_init
- __aeabi_float_init
- __aeabi_mem_init - ROM memset, ROM memcpy, ROM memset4, ROM memcpy44

init functions (these are not essential):

- SystemInit
- boot_double_tap_check
- _retrieve_unique_id_on_boot

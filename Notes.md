# Boot process

memmap_default.ld will setup the memory layout
_entry_point in crt0.S is called
_reset_handler is called with initializates .data section and clears .bss
runtime_init()
main()
exit()

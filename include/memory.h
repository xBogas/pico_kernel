#ifndef W_KERNEL_MEMORY_H
#define W_KERNEL_MEMORY_H

#include <stdlib.h>
#include <stdint.h>


// first address in memory -> validate check linker file
extern char end[];

// allocate kernel memory
// if failed return null
// and set errno ERROR CODE
static void* k_malloc(uint32_t size)
{
	return malloc(size);
}

static void k_free(void* ptr)
{
	if (ptr)
		free(ptr);
}

#endif
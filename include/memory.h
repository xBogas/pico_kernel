#ifndef W_KERNEL_MEMORY_H
#define W_KERNEL_MEMORY_H

#include <stdlib.h>
#include <stdint.h>


// first address in memory -> validate check linker file
extern char end[];

void* k_malloc(size_t size);

void k_free(void* ptr);



#endif
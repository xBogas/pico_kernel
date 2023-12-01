#ifndef W_KERNEL_MEMORY_H
#define W_KERNEL_MEMORY_H

#include <stdlib.h>
#include <stdint.h>

void k_mem_init(void);

void *k_malloc(void);

void k_free(void* ptr);

void *new_mpu_zone(uint8_t access, uint32_t size);

void *expand_mpu_zone(uint8_t id, uint32_t add_len);

#endif
#ifndef W_KERNEL_MEMORY_H
#define W_KERNEL_MEMORY_H

#include "kernel.h"

#ifdef KERNEL_USE_MPU
#define MPU_AP_NO_ACCESS 0x00	// No access
#define MPU_AP_PRIV 0x01		// Privileged access only
#define MPU_AP_U_RO 0x02		// User read-only
#define MPU_AP_FULL_ACCESS 0x03 // Full access
// #define MPU_AP_RESERVED		0x04 // Reserved
#define MPU_AP_PRIV_RO 0x05 // Privileged read-only
#define MPU_AP_R0 0x06		// Privileged and user read-only
#endif

#define PAGE_SIZE 			1024 // 0x400

#define PG_ROUND_UP(x) 		((x + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PG_ROUND_DOWN(x)	((x) & ~(PAGE_SIZE - 1))

#define N_PAGES(x)			PG_ROUND_UP(x) / PAGE_SIZE


void k_mem_init(void);
void *k_malloc(void);
void k_free(void *ptr);
void *new_mpu_zone(uint8_t access, uint32_t size);
void *expand_mpu_zone(uint8_t id, uint32_t add_len);

// These are supposed to be user functions
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
#endif
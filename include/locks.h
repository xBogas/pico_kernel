#ifndef W_KERNEL_LOCKS_H
#define W_KERNEL_LOCKS_H

#include "pico.h"

// Spinlock
struct spinlock
{
	// using hw spinlock
	volatile uint32_t *lock;
	uint32_t irq;

//#ifdef DEBUG_KERNEL
	const char* name;
	uint32_t cpu;
//#endif
};

/**
 * @brief Initialize a spinlock
 * 
 * @param lock object to initialize
 * @param name must not be a temporary variable
 */
void init_lock(struct spinlock *lock, const char *name);

/**
 * @brief Acquire a spinlock
 * spins until lock is acquired
 * 
 * @param lock 
 */
void acquire_lock(struct spinlock *lock);

/**
 * @brief Release a spinlock
 * 
 * @param lock 
 */
void release_lock(struct spinlock *lock);

#endif
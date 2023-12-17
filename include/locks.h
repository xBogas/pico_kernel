#ifndef W_KERNEL_LOCKS_H
#define W_KERNEL_LOCKS_H

#include "kernel.h"

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

/**
 * @brief Check if a spinlock is held
 * 
 * @param lock 
 * @return int 
 */
int holding(struct spinlock *lock);


#define MUTEX_UNOWNED 0xFF

struct mutex
{
	struct spinlock sp_lk;
	uint8_t lock;
	uint8_t owner;
};

void init_mutex(struct mutex *mtx, const char *name);


void acquire_mutex(struct mutex *mtx);


void release_mutex(struct mutex *mtx);
#endif
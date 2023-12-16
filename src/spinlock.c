#include "locks.h"
#include "hardware/structs/sio.h"
#include "hardware/sync.h"

#define BASE_SPINLOCK 		SIO_BASE + SIO_SPINLOCK0_OFFSET
#define SPINLOCK_STATE 		SIO_BASE + SIO_SPINLOCK_ST_OFFSET

void init_lock(struct spinlock *lock, const char *name)
{
	lock->lock = spin_lock_init(spin_lock_claim_unused(1));
	lock->irq = 0;
	lock->name = name;
	lock->cpu = -1;
}

void acquire_lock(struct spinlock *lock)
{
	lock->irq = spin_lock_blocking(lock->lock);
	lock->cpu = cpu_id();
}

void release_lock(struct spinlock *lock)
{
	spin_unlock(lock->lock, lock->irq);
	lock->irq = 0;
	lock->cpu = -1;
}

inline int holding(struct spinlock *lock)
{
	return is_spin_locked(lock->lock);
}
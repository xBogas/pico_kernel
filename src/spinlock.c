#include "locks.h"
#include "pico/sync.h"

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
	lock->cpu = get_core_num();
}

void release_lock(struct spinlock *lock)
{
	spin_unlock(lock->lock, lock->irq);
	lock->irq = 0;
	lock->cpu = -1;
}
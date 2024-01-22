#include "locks.h"
#include "terminal.h"
#include "scheduler.h"
#include "hardware/structs/sio.h"
#include "hardware/sync.h"


void init_lock(struct spinlock *lock, const char *name)
{
	if (lock == NULL)
		panic("init_lock: lock is NULL\n");
	
	int id = spin_lock_claim_unused(1);
	if (id < 0)
		panic("init_lock: no more locks available\n");

	lock->lock = spin_lock_init(id);
	lock->name = name;
	lock->irq = 0;
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
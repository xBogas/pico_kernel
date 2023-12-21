#include "locks.h"
#include "scheduler.h"

void init_mutex(struct mutex *mtx, const char *name)
{
	printk("initializing mutex %s\n", name);
	init_lock(&mtx->sp_lk, name);
	mtx->owner = MUTEX_UNOWNED;
	mtx->blocked = NULL;
}

void acquire_mutex(struct mutex *mtx)
{
	if (!mtx)
		panic("acquire_mutex: mutex is NULL\n");

	acquire_lock(&mtx->sp_lk);
	while (mtx->lock)
		sleep(mtx);

	mtx->lock = 1;
	mtx->owner = mythread();
	release_lock(&mtx->sp_lk);
}


void release_mutex(struct mutex *mtx)
{
	acquire_lock(&mtx->sp_lk);
	mtx->lock = 0;
	mtx->owner = MUTEX_UNOWNED;
	wake_up(mtx);
	release_lock(&mtx->sp_lk);
}
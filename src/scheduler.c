#include "scheduler.h"

// change name this sucks
struct scheduler {
	struct thread_handle *curr;
	struct scheduler *next;
	struct scheduler *prev;
};

static struct scheduler th_list = {
	.curr = NULL,
	.next = NULL,
	.prev = NULL
};

void list_add(struct scheduler *new,
			struct scheduler *prev,
			struct scheduler *next)
{
	new->next = next;
	new->prev = prev;
	next->prev = new;
	prev->next = new;
}

#include <stdio.h>

//! just adding in the end for now
void* sched_add_thread(struct thread_handle* th)
{
	printf("Inserting thread %d\n", th->id);
	if (!th_list.curr) {
		th_list.curr = th;
		th_list.next = &th_list;
		th_list.prev = &th_list;
		return th;
	}

	struct scheduler *new = k_malloc(sizeof(struct scheduler));
	if (!new) {
		k_free(th);
		return NULL;
	}

	new->curr = th;

	// lock_list();
	list_add(new, &th_list, th_list.next);
	// unlock_list();

	struct scheduler *iter = &th_list;
	int i = 0;
	while (iter != NULL) {
		if (i > 7)
			break;

		if (iter->curr->id == 1 && i != 0)
			break;
		
		i++;
		iter = iter->next;
	}
	return th;
}

void start_sched(void)
{
	printf("Starting scheduler\n");
	const struct scheduler *iter = &th_list;
	const struct thread_handle *thread = NULL;

	while (iter->curr)
	{
		thread = iter->curr;
		if (!thread->thread_fn)
			ERROR("thread callback NULL");

		printf("running [%s]\n", thread->name);
		thread->thread_fn(thread->data);
		iter = iter->next;
		delay(100);
	}
	printf("scheduler ended\n");
}

// check if sched started running 
int sched_runtime(void)
{
	return 0;
}

// not sure how to do this for now
// this should wake up thread
void wake_up_thread(struct thread_handle* th)
{
	// wait for th to start running
}
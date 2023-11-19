#include "memory.h"

void* k_malloc(size_t size)
{
	return malloc(size);	
}

void k_free(void* ptr)
{
	free(ptr);
}

// allocate kernel memory
// if failed return null
// and set errno ERROR CODE
void* malloc(size_t size)
{
	return NULL;//malloc(size);
}

void* calloc(size_t count, size_t size)
{
	return NULL;
}

void* realloc(void* ptr, size_t size)
{
	return NULL;
}

void free(void* ptr)
{
	/* if (ptr)
		free(ptr); */
}



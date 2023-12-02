#include "memory.h"
#include "locks.h"
#include "terminal.h"
#include "kernel.h"
#include <string.h>

#define PAGE_SIZE 			1024 // 0x400

#define PG_ROUND_UP(x) 		((x + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PG_ROUND_DOWN(x)	((x) & ~(PAGE_SIZE - 1))

#define N_PAGES(x)			PG_ROUND_UP(x) / PAGE_SIZE

void free_range(void *start, void *end);
void setup_mpu(void);

struct next {
	struct next *next;
};

static struct {
	struct spinlock lock;
	struct next *free;
} k_mem;

// Values generated by the linker
extern char end[];
extern char __StackLimit[];

void k_mem_init(void)
{
	k_mem.free = NULL;
	init_lock(&k_mem.lock, "mem lock");
	free_range(end, __StackLimit);

	setup_mpu();
	printk("mpu setup done\n");
}

void free_range(void *start, void *end)
{
	char *p;
	if ((uint32_t)end % PAGE_SIZE == 0)
		end--;
	
	p = (char *)PG_ROUND_DOWN((uint32_t)end);

	for (; p - PAGE_SIZE > (char *)start; p -= PAGE_SIZE)
		k_free(p);

	printk("will lose %d bytes\n", (uint32_t)p - (uint32_t)start);
}

void k_free(void *ptr)
{
	if (((uint32_t)ptr % PAGE_SIZE) != 0 || (char *)ptr < end || (char *)ptr >= __StackLimit)
		panic("k_free: bad page ptr %p\n", ptr);


	memset(ptr, 1, PAGE_SIZE);
	struct next *mem = (struct next *)ptr;
	mem->next = NULL;

	acquire_lock(&k_mem.lock);
	if (!k_mem.free) {
		k_mem.free = mem;
		release_lock(&k_mem.lock);
		return;
	}

	if (mem < k_mem.free) {
		mem->next = k_mem.free;
		k_mem.free = mem;
		release_lock(&k_mem.lock);
		return;
	}

	struct next *iter = k_mem.free;
	while (iter->next != NULL) {
		if (iter->next > mem)
			break;				
		iter = iter->next;
	}
	mem->next = iter->next;
	iter->next = mem;
	release_lock(&k_mem.lock);
}

// allocate a page of kernel memory
void *k_malloc(void)
{
	struct next *mem;

	acquire_lock(&k_mem.lock);
	mem = k_mem.free;
	if (mem)
		k_mem.free = mem->next;
	release_lock(&k_mem.lock);

	if (mem)
		memset(mem, 0, PAGE_SIZE);
	return (void *)mem;
}

// allocate multiple consecutive pages
static void *k_malloc_pages(size_t pages)
{
	acquire_lock(&k_mem.lock);
	if (!k_mem.free) {
		release_lock(&k_mem.lock);
		return NULL;
	}

	struct next *start 	= k_mem.free;
	struct next *prev 	= start;
	struct next *iter 	= start;

	while (iter->next != NULL) {
		// non consecutive pages
		if ((iter->next - iter) != PAGE_SIZE) {
			prev = iter;
			start = iter->next;
			iter = start;
			continue;
		}

		// check if we have enough pages
		if (iter - start == (pages-1) * PAGE_SIZE) {
			prev->next = iter->next;
			release_lock(&k_mem.lock);
			memset(start, 0, pages * PAGE_SIZE);
			return start;
		}

		iter = iter->next;
	}

	release_lock(&k_mem.lock);
	return NULL;
}

/**
 * Allocate len consecutive pages aligned to addr
 */
static void *k_malloc_aligned(size_t len, size_t addr)
{
	acquire_lock(&k_mem.lock);
	if (!k_mem.free) {
		release_lock(&k_mem.lock);
		return NULL;
	}

	struct next *start 	= k_mem.free;
	struct next *prev 	= start;
	struct next *iter 	= start;

alignment:
	while ((uint32_t)start % addr != 0) {
		prev = start;
		start = start->next;
		if (!start) {
			release_lock(&k_mem.lock);
			return NULL;
		}
	}
	iter = start;

	while (iter->next != NULL) {
		// not consecutive pages restart
		if ((uint32_t)iter->next - (uint32_t)iter != PAGE_SIZE) {
			prev = iter;
			start = iter->next;
			iter = start;
			goto alignment;
			return NULL;
		}

		// check if we have enough pages
		if (((uint32_t)iter - (uint32_t)start) == (len-1) * PAGE_SIZE) {
			prev->next = iter->next;
			release_lock(&k_mem.lock);
			memset(start, 0, len * PAGE_SIZE);
			return start;
		}

		iter = iter->next;
	}

	release_lock(&k_mem.lock);
	return NULL;
}

// allocate kernel memory
// if failed return null
// and set errno ERROR CODE
void *malloc(size_t size)
{
	return NULL; // malloc(size);
}

void *calloc(size_t count, size_t size)
{
	return NULL;
}

void *realloc(void *ptr, size_t size)
{
	return NULL;
}

void free(void *ptr)
{
}

#include "hardware/structs/mpu.h"
#include "pico/stdlib.h"
#include "hardware/sync.h"
#include "RP2040.h"

struct mpu_data {
	uint32_t addr;		// Region base address
	uint32_t access;	// Access permissions
	uint32_t len;		// Region size bytes
};

// only 8 regions are supported
static struct mpu_data k_mpu[8];

void setup_mpu(void)
{
	__disable_irq();
	ARM_MPU_Disable();

	// set up the default region as privileged
	ARM_MPU_Enable(0x04);
	__enable_irq();
}

uint8_t translate_size(uint32_t size)
{
	int valid = 32;
	for (size_t i = ARM_MPU_REGION_SIZE_32B; i < ARM_MPU_REGION_SIZE_128KB; i++) {
		if (size == valid)
			return i;
		valid *= 2;
	}

	return 0;
}

void update_mpu(uint32_t id, struct mpu_data *zone, uint8_t status)
{
	uint8_t size = translate_size(zone->len);
	mpu_hw->rnr = id;
	mpu_hw->rbar = (uint32_t)zone->addr;
	mpu_hw->rasr = 	zone->access	<< MPU_RASR_AP_Pos		|
					size			<< MPU_RASR_SIZE_Pos	|
					status			<< MPU_RASR_ENABLE_Pos;
}

void *new_mpu_zone(uint8_t access, uint32_t len)
{
	static uint8_t region_id = 0;
	if (region_id == 7)
		return NULL;

	uint8_t size = translate_size(len);
	if (!size) {
		printk("Invalid mpu size %d\n", len);
		return NULL;
	}

	if (len < PAGE_SIZE) {
		printk("Requested mpu zone < PAGE_SIZE\n");
		len = PAGE_SIZE;
	}

	void *zone = k_malloc_aligned(len / PAGE_SIZE, len);
	if (!zone) {
		printk("Failed to allocate mpu zone\n");
		return NULL;
	}

	struct mpu_data *mpu_zone = &k_mpu[region_id];
	mpu_zone->addr = (uint32_t)zone;
	mpu_zone->access = access;
	mpu_zone->len = len;

	update_mpu(region_id++, mpu_zone, 1);
	return zone;
}

/**
	Update mpu expansion.
 */
void *expand_mpu_zone(uint8_t id, uint32_t add_len)
{
	if (unlikely(id > 7)) 
		return NULL;

	if (unlikely(add_len == 0))
		return NULL;

	if (unlikely(!k_mpu[id].addr))
		return NULL;

	uint32_t len = k_mpu[id].len + add_len;	
	if (k_mpu[id].addr % len == 0) {
		// request expansion of the same zone 
		void *zone = k_malloc_aligned(N_PAGES(add_len), k_mpu[id].addr + k_mpu[id].len);
		if (!zone)
			return NULL;

		k_mpu[id].len += PG_ROUND_UP(add_len);
		update_mpu(id, &k_mpu[id], 1);
		return (void *)k_mpu[id].addr;
	}

	// memory is not aligned
	// memory needs to be reallocated
	free_range((void*)k_mpu[id].addr, (void*)(k_mpu[id].addr + k_mpu[id].len -1));
	void *zone = k_malloc_aligned(N_PAGES(len), len);
	if (!zone) {
		printk("Failed to expand mpu zone\n");
		// try to allocate previous size
		zone = k_malloc_aligned(k_mpu[id].len / PAGE_SIZE, k_mpu[id].len);
		if (!zone)
			panic("mpu zone became invalid\n");

		k_mpu[id].addr = (uint32_t)zone;
		update_mpu(id, &k_mpu[id], 1);
		return zone;
	}

	k_mpu[id].addr = (uint32_t)zone;
	k_mpu[id].len = PG_ROUND_UP(len);
	return (void *)k_mpu[id].addr;
}
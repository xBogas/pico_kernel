#include "memory.h"
#include "locks.h"
#include "terminal.h"
#include <string.h>

#define PAGE_SIZE 256

#define PAGE_ALIGN(x) ((x + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

void free_range(void *start, void *end);
void setup_mpu(void);

struct next {
	struct next *next;
};

struct {
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
	p = (char *)PAGE_ALIGN((uint32_t)end) - PAGE_SIZE;

	for (; p - PAGE_SIZE > (char *)start; p -= PAGE_SIZE)
		k_free(p);

	printk("will lose %d bytes\n", (uint32_t)p - (uint32_t)start);
}

void k_free(void *ptr)
{
	struct next *mem;

	if (((uint32_t)ptr % PAGE_SIZE) != 0 || (char *)ptr < end || (char *)ptr >= __StackLimit)
		panic("k_free: bad ptr %p\n", ptr);

	memset(ptr, 1, PAGE_SIZE);
	mem = (struct next *)ptr;

	acquire_lock(&k_mem.lock);
	mem->next = k_mem.free;
	k_mem.free = mem;
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

static void *k_malloc_pages(size_t pages)
{
	struct next *mem = k_malloc();
	if (!mem)
		return NULL;

	for (size_t i = 1; i < pages; i++) {
		struct next *last = k_malloc();
		if (!last) {
			free_range(mem, mem->next);
			return NULL;
		}
		mem->next = last;
	}

	return mem;
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

static uint8_t region_id;

struct mpu_data {
	uint32_t access;
	uint32_t size;
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

int translate_size(uint32_t size)
{
	int valid = 32;
	for (size_t i = ARM_MPU_REGION_SIZE_32B; i < ARM_MPU_REGION_SIZE_128KB; i++) {
		if (size == valid)
			return i;
		valid *= 2;
	}

	return 0;
}

void *new_mpu_zone(uint8_t access, uint32_t len)
{
	int size = translate_size(len);
	if (!size) {
		printk("Invalid mpu size %d\n", len);
		return NULL;
	}

	if (len < PAGE_SIZE)
		panic("Requested mpu zone < PAGE_SIZE\n");

	void *zone = k_malloc_pages(len / PAGE_SIZE);

	if (!zone)
		return NULL;

	if (region_id == 7)
		return NULL;

	k_mpu[region_id] = (struct mpu_data){
		.access = access,
		.size = size
	};

	// set up the mpu region
	mpu_hw->rnr = (uint32_t)region_id++;
	mpu_hw->rbar = (uint32_t)zone;
	mpu_hw->rasr = access << MPU_RASR_AP_Pos |
				   ARM_MPU_REGION_SIZE_256B << MPU_RASR_SIZE_Pos |
				   1 << MPU_RASR_ENABLE_Pos;

#if PAGE_SIZE != 256
#error "MPU region size must be configured"
#endif

	return zone;
}

void *expand_mpu_zone(uint8_t id)
{
	return NULL;
}
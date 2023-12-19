#ifndef W_KERNEL_SIO_FIFO_H_INCLUDED
#define W_KERNEL_SIO_FIFO_H_INCLUDED

#include "kernel.h"
#include "hardware/structs/sio.h"
#include "hardware/sync.h"

/**
 * Check if can read from FIFO
 */
static inline uint32_t fifo_r(void)
{
	return sio_hw->fifo_st & SIO_FIFO_ST_VLD_BITS;
}

/**
 * Read from FIFO
 */
static inline uint32_t fifo_read(void)
{
	return sio_hw->fifo_rd;
}

/**
 * Flush FIFO
 */
static inline void fifo_flush(void)
{
	while (fifo_r())
		fifo_read();
}

/**
 * Pop from FIFO
 * Blocks until there is data ready
 */
static inline uint32_t fifo_pop(void)
{
	while (!fifo_r())
		;
	return fifo_read();
}

/**
 * Check if can write to FIFO
 */
static inline uint32_t fifo_w(void)
{
	return sio_hw->fifo_st & SIO_FIFO_ST_RDY_BITS;
}

/**
 * Write to FIFO
 */
static inline void fifo_write(uint32_t data)
{
	sio_hw->fifo_wr = data;
}

/**
 * Push to FIFO
 * Blocks until there is space
 */
static inline void fifo_push(uint32_t data)
{
	while (!fifo_w())
		;
	fifo_write(data);
	__sev();
}

#endif
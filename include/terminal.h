#ifndef W_KERNEL_TERMINAL_H_INCLUDED
#define W_KERNEL_TERMINAL_H_INCLUDED

#include <stdarg.h>

/**
 * @brief Panics the terminal
 * 
 */
void panic_terminal();

/**
 * @brief Initializes the terminal
 * 
 */
void term_init(void);

/**
 * @brief Prints a string to the terminal
 * 
 * @param str The string to print
 * @param ... The arguments to the string
 */
void printk(const char *str, ...);

/**
 * @brief Prints a string to the terminal
 * even if the kernel is panicked
 * 
 * @param str The string to print
 * @param ... The arguments to the string
 */
void force_printk(const char *str, ...);

//TODO Read from terminal

#if KERNEL_CONSOLE_USB

/**
 * @brief Checks if the terminal is connected
 * 
 * @return int 1 if connected, 0 otherwise
 */
int term_connected(void);

/**
 * @brief USB tud_task to be called repeatedly
 */
void usb_task(void);

#endif

#endif
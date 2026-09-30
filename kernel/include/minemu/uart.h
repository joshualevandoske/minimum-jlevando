#ifndef MINEMU_UART_H
#define MINEMU_UART_H

#include <stdint.h>

void minemu_uart_init(void);

void minemu_uart_putc(uint8_t byte);
void minemu_uart_puts(const char *string);

void minemu_uart_irq_handler(void);

int minemu_uart_getc(uint8_t *byte);

#endif
#include <stddef.h>
#include <stdint.h>

#include "minemu/irq.h"
#include "minemu/platform.h"
#include "minemu/uart.h"

#define UART_RX_BUFFER_CAPACITY 128

static uint8_t rx_buffer[UART_RX_BUFFER_CAPACITY];
static size_t rx_head;
static size_t rx_tail;
static size_t rx_count;

void minemu_uart_putc(uint8_t byte) {
    while (!(MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY)) {
    }

    MINEMU_UART0->tx_data = (uint32_t)byte;
}

void minemu_uart_puts(const char *string) {
    while (*string != '\0') {
        minemu_uart_putc((uint8_t)*string);
        ++string;
    }
}

void minemu_uart_irq_handler(void) {
    while (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint8_t byte = (uint8_t)MINEMU_UART0->rx_data;

        if (rx_count < UART_RX_BUFFER_CAPACITY) {
            rx_buffer[rx_head] = byte;
            rx_head = (rx_head + 1) % UART_RX_BUFFER_CAPACITY;
            ++rx_count;
        }
    }
}

int minemu_uart_getc(uint8_t *byte) {
    int available = 0;

    minemu_irq_disable();

    if (rx_count > 0) {
        *byte = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % UART_RX_BUFFER_CAPACITY;
        --rx_count;
        available = 1;
    }

    minemu_irq_enable();

    return available;
}

void minemu_uart_init(void) {
    MINEMU_INTERRUPT->enable |=
        (UINT32_C(1) << MINEMU_IRQ_UART0);

    MINEMU_UART0->control |=
        MINEMU_UART_CONTROL_RX_IRQ_ENABLE;

    minemu_irq_enable();
}
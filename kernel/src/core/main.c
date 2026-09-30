#include "minemu/boot.h"
#include "minemu/irq.h"
#include "minemu/platform.h"
#include "minemu/trap.h"
#include "minemu/trace.h"
#include "minemu/uart.h"

void minemu_kernel_main(const struct minemu_boot_info *boot_info) {
    if ((uintptr_t)boot_info != MINEMU_BOOT_INFO_VADDR ||
        boot_info->magic != MINEMU_BOOT_INFO_MAGIC ||
        boot_info->version != MINEMU_ABI_VERSION ||
        boot_info->size != sizeof(*boot_info) ||
        boot_info->system_rom_base != UINT32_C(0x08000000) ||
        boot_info->direct_map_vaddr != UINT32_C(0xc0000000) ||
        boot_info->direct_map_paddr != UINT32_C(0x40000000) ||
        boot_info->direct_map_size != UINT32_C(0x04000000)) {
        minemu_trace_event(UINT32_C(0xb007bad0));
        minemu_fail_stop();
    }
    minemu_uart_puts("hello world\n");

    minemu_uart_init();
    
    uint8_t byte;

    char line[21];
    size_t line_length = 0;

    minemu_uart_puts("msh> ");

    for (;;) {
        if (!minemu_uart_getc(&byte)) {
            continue;
        }

        if (byte == '\n') {
            line[line_length] = '\0';

            size_t start = 0;

            while (line[start] == ' ') {
                ++start;
            }

            if (line[start] == '\0') {
                line_length = 0;
                minemu_uart_puts("msh> ");
                continue;
            }

            size_t command_end = start;

            while (line[command_end] != ' ' &&
                line[command_end] != '\0') {
                ++command_end;
            }

            if (command_end - start == 4 &&
                line[start] == 'e' &&
                line[start + 1] == 'c' &&
                line[start + 2] == 'h' &&
                line[start + 3] == 'o') {

                size_t arg = command_end;

                while (line[arg] == ' ') {
                    ++arg;
                }

                minemu_uart_puts(&line[arg]);
                minemu_uart_putc('\n');
            }
            else {
                line[command_end] = '\0';

                minemu_uart_puts("command not found: ");
                minemu_uart_puts(&line[start]);
                minemu_uart_putc('\n');
            }

            line_length = 0;
            minemu_uart_puts("msh> ");
        }
        else if (byte == 0x08 || byte == 0x7f) {
            if (line_length > 0) {
                --line_length;
            }
        }
        else {
            if (line_length < 20) {
                line[line_length] = (char)byte;
                ++line_length;
            }
        }
    }
}

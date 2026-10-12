
#include <stdbool.h>
#include <stdint.h>

#include "minemu/user_abi.h"

#define MAX_LINE_LENGTH 64U

// Write bytes to the console through the kernel.
static void uart_write(const char *text, uint32_t length) {
    if (length > 0U) {
        (void)ioctl(MINEMU_FD_UART_OUT,
                    MINEMU_IOCTL_UART_WRITE,
                    (void *)text, length);
    }
}

// Replacement for the HW1 minemu_uart_puts().
static void uart_puts(const char *text) {
    uint32_t length = 0;

    while (text[length] != '\0') {
        ++length;
    }

    uart_write(text, length);
}

// Interpret a completed command.
static void execute_command(char *line) {
    uint32_t start = 0;

    // Ignore leading spaces.
    while (line[start] == ' ') {
        ++start;
    }

    // Empty command.
    if (line[start] == '\0') {
        return;
    }

    // Find the end of the command name.
    uint32_t end = start;

    while (line[end] != ' ' && line[end] != '\0') {
        ++end;
    }

    // Recognize echo.
    if (end - start == 4U &&
        line[start] == 'e' &&
        line[start + 1U] == 'c' &&
        line[start + 2U] == 'h' &&
        line[start + 3U] == 'o') {

        uint32_t arg = end;

        // Skip spaces before the argument.
        while (line[arg] == ' ') {
            ++arg;
        }

        uint32_t finish = arg;

        while (line[finish] != '\0') {
            ++finish;
        }

        // Remove trailing spaces.
        while (finish > arg && line[finish - 1U] == ' ') {
            --finish;
        }

        uart_write(&line[arg], finish - arg);
        uart_puts("\n");
    } else {
        // Unknown command: display only its first word.
        uart_puts("command not found: ");
        uart_write(&line[start], end - start);
        uart_puts("\n");
    }
}

void minemu_user_main(void) {
    char line[MAX_LINE_LENGTH + 1U];
    uint32_t line_length = 0;
    bool discard_line = false;

    uart_puts("msh> ");

    for (;;) {
        uint8_t byte = 0;

        // Read one character through the syscall interface.
        int32_t count = ioctl(MINEMU_FD_UART_IN,
                              MINEMU_IOCTL_UART_READ,
                              (void *)&byte, UINT32_C(1));

        if (count <= 0) {
            continue;
        }

        if (byte == '\n') {
            if (!discard_line) {
                line[line_length] = '\0';
                execute_command(line);
            }

            line_length = 0;
            discard_line = false;
            static char prompt[] = "msh> ";

            (void)ioctl(MINEMU_FD_UART_OUT,
                        MINEMU_IOCTL_UART_WRITE,
                        (void *)prompt,
                        (uint32_t)(sizeof(prompt) - 1U));
                    }
        else if (byte == 0x08U || byte == 0x7fU) {
            // Backspace or delete.
            if (!discard_line && line_length > 0U) {
                --line_length;
            }
        }
        else if (!discard_line) {
            // Collapse repeated spaces.
            if (byte == ' ' &&
                (line_length == 0U ||
                 line[line_length - 1U] == ' ')) {
                continue;
            }

            if (line_length < MAX_LINE_LENGTH) {
                line[line_length++] = (char)byte;
            } else {
                // Discard overlong input until newline.
                discard_line = true;
            }
        }
    }
}

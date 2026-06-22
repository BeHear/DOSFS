#include "vga.h"
#include "../include/io.h"
#include "../libc/string.h"
#include <stdarg.h>

static uint16_t* vga_buffer;
static int vga_row;
static int vga_col;
static uint8_t vga_color_attr;

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

void vga_init(void) {
    vga_buffer = (uint16_t*)0xB8000;
    vga_row = 0;
    vga_col = 0;
    vga_color_attr = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_clear();
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = vga_entry(' ', vga_color_attr);
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_color_attr = vga_entry_color(fg, bg);
}

void vga_scroll(void) {
    if (vga_row >= VGA_HEIGHT) {
        for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
            vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
        }
        for (int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
            vga_buffer[i] = vga_entry(' ', vga_color_attr);
        }
        vga_row = VGA_HEIGHT - 1;
    }
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(' ', vga_color_attr);
        }
    } else {
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_color_attr);
        vga_col++;
    }

    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }

    vga_scroll();

    uint16_t cursor = vga_row * VGA_WIDTH + vga_col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(cursor & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((cursor >> 8) & 0xFF));
}

void vga_puts(const char* s) {
    while (*s) {
        vga_putchar(*s++);
    }
}

void vga_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char buf[32];

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    if (!s) s = "(null)";
                    vga_puts(s);
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    itoa(val, buf, 10);
                    vga_puts(buf);
                    break;
                }
                case 'u': {
                    uint32_t val = va_arg(args, uint32_t);
                    uitoa(val, buf, 10);
                    vga_puts(buf);
                    break;
                }
                case 'x': {
                    uint32_t val = va_arg(args, uint32_t);
                    uitoa(val, buf, 16);
                    vga_puts(buf);
                    break;
                }
                case 'p': {
                    uint32_t val = va_arg(args, uint32_t);
                    vga_puts("0x");
                    uitoa(val, buf, 16);
                    vga_puts(buf);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    vga_putchar(c);
                    break;
                }
                case '%': {
                    vga_putchar('%');
                    break;
                }
                default:
                    vga_putchar('%');
                    vga_putchar(*fmt);
                    break;
            }
        } else {
            vga_putchar(*fmt);
        }
        fmt++;
    }

    va_end(args);
}

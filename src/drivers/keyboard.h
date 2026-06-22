#ifndef DANYA_KEYBOARD_H
#define DANYA_KEYBOARD_H

#include "../kernel/idt.h"

void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_has_key(void);
uint8_t keyboard_get_scancode(void);

#endif

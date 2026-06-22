#ifndef DANYA_TIMER_H
#define DANYA_TIMER_H

#include "../kernel/idt.h"

void timer_init(uint32_t freq);
uint32_t timer_get_ticks(void);

#endif

#ifndef TIMER_H
#define TIMER_H
#include "common/common.h"

void timer_init(uint16_t bps);
uint32_t timer_get_ticks();
void timer_consume_tick();
uint8_t timer_has_ticks();
void timer_reset_ticks();

#endif /* TIMER_H */

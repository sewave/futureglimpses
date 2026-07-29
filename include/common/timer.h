#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

typedef enum {
    TIMER_SPEED_SLOWEST,
    TIMER_SPEED_SLOW,
    TIMER_SPEED_NORMAL,
    TIMER_SPEED_FAST,
    TIMER_SPEED_ULTRA,
    TIMER_SPEED_COUNT
} TimerSpeedType;

void timer_init(uint16_t bps);
uint32_t timer_get_ticks();
uint8_t timer_has_ticks();
void timer_reset_ticks();
void timer_set_speed(TimerSpeedType speed);
TimerSpeedType timer_get_speed();

#endif /* TIMER_H */

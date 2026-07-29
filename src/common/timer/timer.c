#include <allegro/internal/alconfig.h>
#include <allegro/timer.h>
#include "common/timer.h"

#define NORMAL_SPEED 4
volatile long logicTicks;
static uint8_t frameTicks;
static TimerSpeedType speed;
static uint8_t speedToFrames[TIMER_SPEED_COUNT] = {1, 2, NORMAL_SPEED, 8, 12};

static void timer_handler_int() {
	logicTicks += frameTicks;
}
END_OF_FUNCTION(timer_handler_int);

void timer_init(uint16_t bps) {
    timer_reset_ticks();
    timer_set_speed(TIMER_SPEED_NORMAL);
    LOCK_VARIABLE(logicTicks);
	LOCK_FUNCTION(timer_handler_int);
	install_int_ex(timer_handler_int, BPS_TO_TIMER(bps));
}

void timer_set_speed(TimerSpeedType newSpeed) {
    speed = newSpeed;
    frameTicks = speedToFrames[speed];
}

TimerSpeedType timer_get_speed() {
    return speed;
}

uint32_t timer_get_ticks() {
    return logicTicks / NORMAL_SPEED;
}

uint8_t timer_has_ticks() {
    return logicTicks >= NORMAL_SPEED;
}

void timer_reset_ticks() {
    logicTicks = 0;
}

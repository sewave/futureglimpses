#include "common/timer.h"
#include <allegro.h>

volatile long logicTicks = 0;

static void timer_handler_int() {
	logicTicks++;
}
END_OF_FUNCTION(timer_handler_int);

void timer_init(uint16_t bps) {
    LOCK_VARIABLE(logicTicks);
	LOCK_FUNCTION(timer_handler_int);
	install_int_ex(timer_handler_int, BPS_TO_TIMER(bps));
}

uint32_t timer_get_ticks() {
    return logicTicks;
}

void timer_consume_tick() {
    if(logicTicks) logicTicks--;
}

uint8_t timer_has_ticks() {
    return logicTicks > 0;
}

void timer_reset_ticks() {
    logicTicks = 0;
}

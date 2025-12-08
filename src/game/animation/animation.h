#ifndef ANIMATION_H
#define ANIMATION_H
#include "../game_lib.h"
#define MOVABLE_UNITS 5

#define BLINK_TIME 35
#define BLINK_MOD 5
#define BLINK_VALUE 1

void game_animation_unit_set(GameUnit *unit);
void game_animation_unit_reset(GameUnit* unit);
void game_animation_unit_advance(GameContext* context, GameUnit* unit);
uint8_t game_animation_unit_finished(GameUnit* unit);

#endif /* ANIMATION_H */

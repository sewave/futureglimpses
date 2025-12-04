#ifndef ANIMATION_H
#define ANIMATION_H
#include "../game_lib.h"
#define MOVABLE_UNITS 5
#define MOVABLE_UNITS_STATES 5

void game_animation_movable_unit_set(GameUnit* unit);
void game_animation_unit_reset(GameUnit* unit);
void game_animation_unit_advance(GameUnit* unit);
uint8_t game_animation_unit_finished(GameUnit* unit);

#endif /* ANIMATION_H */

#ifndef ANIMATION_H
#define ANIMATION_H
#include "../game_lib.h"
#define MOVABLE_UNITS 5
#define BUILDING_UNITS UNIT_TYPE_NUMBER - MOVABLE_UNITS

#define BLINK_TIME 35
#define BLINK_MOD 10
#define BLINK_FRAMES 5

void game_animation_unit_set(GameUnit *unit);
void game_animation_reset(AnimationStatus* animationStatus);
void game_animation_unit_advance(GameContext* context, GameUnit* unit);
void game_animation_object_advance(GameContext* context, Object* object);
uint8_t game_animation_finished(AnimationStatus* animationStatus);
void game_animation_object_set(Object *object);

#endif /* ANIMATION_H */

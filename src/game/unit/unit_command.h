#ifndef UNIT_COMMAND_H
#define UNIT_COMMAND_H
#include "../game_lib.h"

void game_unit_command_idle(GameUnit* unit);
void game_unit_command_attack(GameUnit *unit, GameUnit *target, UnitStateEnum nextState);
void game_unit_command_defend(GameUnit* unit);
void game_unit_command_move(GameUnit* unit, GameUnit* target, int16_t targetX,  int16_t targetY);
void game_unit_command_move_attack(GameUnit* unit, GameUnit* target, int16_t targetX,  int16_t targetY);
void game_unit_command_move_anim(GameUnit *unit, UnitStateEnum nextState, uint16_t totalAnimationTime);

#endif /* UNIT_COMMAND_H */

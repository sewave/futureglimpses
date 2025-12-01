#ifndef UNIT_COMMAND_H
#define UNIT_COMMAND_H
#include "../game_lib.h"

void game_unit_command_idle(GameUnit* unit);
void game_unit_command_attack(GameUnit* unit, GameUnit* target);
void game_unit_command_defend(GameUnit* unit);
void game_unit_command_move(GameUnit* unit, GameUnit* target, int32_t targetX,  int32_t targetY);
void game_unit_command_move_attack(GameUnit* unit, GameUnit* target, int32_t targetX,  int32_t targetY);
void game_unit_command_move_anim(GameUnit* unit);

#endif /* UNIT_COMMAND_H */

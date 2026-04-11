#ifndef UNIT_COMMAND_H
#define UNIT_COMMAND_H
#include "game/game_lib.h"

void game_unit_command_idle(GameUnit* unit);
void game_unit_command_attack(GameUnit *unit, GameUnit *target, UnitStateEnum nextState);
void game_unit_command_defend(GameUnit* unit);
void game_unit_command_move(GameUnit* unit, GameUnit* target, int16_t targetX,  int16_t targetY);
void game_unit_command_move_attack(GameUnit* unit, GameUnit* target, int16_t targetX,  int16_t targetY);
void game_unit_command_set_move_anim(GameUnit *unit, UnitStateEnum nextState);
void game_unit_command_work(GameUnit* worker, GameUnit* target, int16_t targetX,  int16_t targetY);
void game_unit_command_move_player(GameContext *context, GameUnit *unit, GameUnit* target, int x, int y);
void game_unit_command_move_attack_player(GameContext *context, GameUnit *unit, GameUnit* target, int x, int y);

#endif /* UNIT_COMMAND_H */

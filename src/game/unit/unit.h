#ifndef UNIT_H
#define UNIT_H
#include "../game_lib.h"

GameUnit *game_unit_get_by_handle(GameContext *gameState, int handle);
void game_unit_destroy(GameContext *gameState, int handle);
void game_units_init(GameContext *gameState);
int game_unit_spawn(GameContext *gameState, GameUnit *unitData);

#endif /* UNIT_H */

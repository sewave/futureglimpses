#ifndef UNIT_H
#define UNIT_H
#include "../game_lib.h"

#define FIRST_UNIT_GENERATION 1

GameUnit* game_unit_get_by_handle(GameState* gameState, int handle);
void game_unit_destroy(GameState* gameState, int handle);
void game_units_init(GameState* gameState);
int game_unit_spawn(GameState* gameState, GameUnit* unitData);

#endif /* UNIT_H */

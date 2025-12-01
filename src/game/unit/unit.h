#ifndef UNIT_H
#define UNIT_H
#include "../game_lib.h"

GameUnit *game_unit_get_by_id(GameContext *context, int handle);
void game_unit_destroy(GameContext *context, int handle);
void game_units_init(GameContext *context);
GameUnit* game_unit_spawn(GameContext *gameContext, GameUnit *unitData);

#endif /* UNIT_H */

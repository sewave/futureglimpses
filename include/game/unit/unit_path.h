#ifndef UNIT_PATH_H
#define UNIT_PATH_H
#include "game/game_lib.h"

uint8_t game_unit_path_find(GameContext* context, GameUnit* unit, uint16_t targetX, uint16_t targetY);
void game_unit_path_init(GameUnit* unit, uint16_t targetX, uint16_t targetY);

#endif /* UNIT_PATH_H */

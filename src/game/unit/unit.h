#ifndef UNIT_H
#define UNIT_H
#include "../game_lib.h"

GameUnit *game_unit_get_by_id(GameContext *context, UnitId handle);
void game_unit_destroy(GameContext *context, UnitId handle);
void game_units_init(GameContext *context);
GameUnit *game_unit_spawn(GameContext *context, UnitTypeEnum type, UnitControllerEnum controller, uint16_t x, uint16_t y);
void game_unit_face_target(GameUnit* unit, GameUnit* target);
void game_unit_damage(GameContext *gameContext, GameUnit* unit, GameUnit* target);

#endif /* UNIT_H */

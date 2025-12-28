#ifndef BUILDING_H
#define BUILDING_H
#include "../game_lib.h"

void building_add_to_train_queue(GameContext* context, GameUnit* building, UnitTypeEnum unitType);
void building_update(GameContext *context, GameUnit *building);
#endif /* BUILDING_H */

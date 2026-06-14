#ifndef BUILDING_H
#define BUILDING_H
#include "game/game.h"

void building_add_to_train_queue(GameContext* context, GameUnit* building, TrainingTypeEnum trainingType);
void building_update(GameContext *context, GameUnit *building);
GameUnit* building_place_building(GameContext *context, UnitTypeEnum buildingType, ControllerEnum controller, uint16_t x, uint16_t y);
void building_handle_placing_input(GameContext *context);
void building_add_construction(GameContext *context, GameUnit *building);
void building_repair(GameContext *context, GameUnit *building);
void building_complete(GameContext *context, GameUnit *building);

#endif /* BUILDING_H */

#include "building.h"

#define NOT_ENOUGH_RESOURCE_TIME SEC_TO_FRAMES(5)
#define QUEUE_FULL_MSG_TIME SEC_TO_FRAMES(5)
#define UNIT_NORMAL_RESOURCES 2
#define SPIRAL_DIRECTIONS 4

static Position spiralDirections[SPIRAL_DIRECTIONS] = {
		{1, 0},
		{0, 1},
		{-1, 0},
		{0, -1}};

static Position game_building_get_spawn_position(GameContext *context, GameUnit *building) {
	// Starting at unit (-1, -1) we spiral right, down, left, up
	int x = building->x - 1;
	int y = building->y - 1;
	for (int spiralSide = building->tileSize + 1;; spiralSide += 2) {
		for (int spiralDir = 0; spiralDir < SPIRAL_DIRECTIONS; spiralDir++) {
			for (int spiralInc = 0; spiralInc < spiralSide; spiralInc++) {
				int clampX = clamp(x, BOARD_X_MIN, BOARD_X_MAX);
				int clampY = clamp(y, BOARD_Y_MIN, BOARD_Y_MAX);
				if (context->walkabilityGrid[clampX][clampY] == WALKABILITY_FREE) {
					return (Position) {clampX, clampY};
				}
				x += spiralDirections[spiralDir].x;
				y += spiralDirections[spiralDir].y;
			}
		}
		x--;
		y--;
	}
}

static uint8_t building_queue_training(GameUnit *building, UnitTypeEnum unitType) {
	BuildingData *buildingData = &building->typed.buildingData;
	if (buildingData->queueNextIndex >= MAX_BUILDING_QUEUE) return FALSE;
	buildingData->queue[buildingData->queueNextIndex++] = unitType;
	return TRUE;
}

static void building_start_training(GameUnit *building, UnitTypeEnum unitType) {
	BuildingData *buildingData = &building->typed.buildingData;
	buildingData->isTraining = TRUE;
	buildingData->trainUnit = unitType;
	buildingData->currentTrainTicks = 0;
	buildingData->targetTrainTicks = game_unit_get_resources(unitType)->time;
}

void building_update(GameContext *context, GameUnit *building) {
	BuildingData *buildingData = &building->typed.buildingData;
	if (buildingData->isTraining) {
		buildingData->currentTrainTicks++;
		if (buildingData->currentTrainTicks >= buildingData->targetTrainTicks) {
            if(resource_has_enough(context, building->controller, RESOURCE_TYPE_AVAILABLE_FOOD,
                    game_unit_get_resources(buildingData->trainUnit)->used[RESOURCE_TYPE_AVAILABLE_FOOD])) {
                Position spawn = game_building_get_spawn_position(context, building);
			    game_unit_spawn(context, buildingData->trainUnit, building->controller, spawn.x, spawn.y);
			    buildingData->isTraining = FALSE;
            }
            else {
                buildingData->currentTrainTicks--;
            }
		}
	}
    else {
		if (buildingData->queueNextIndex) {
			building_start_training(building, buildingData->queue[0]);
			buildingData->queueNextIndex--;
			for (int i = 0; i < buildingData->queueNextIndex; i++) {
				buildingData->queue[i] = buildingData->queue[i + 1];
			}
		}
	}
}

void building_add_to_train_queue(GameContext *context, GameUnit *building, UnitTypeEnum unitType) {
	// Check funds and food
    UnitResourcesData* unitResources = game_unit_get_resources(unitType);
	for (int i = 0; i < UNIT_USED_RESOURCES; i++) {
		if (!resource_has_enough(context, building->controller, i, unitResources->used[i])) {
			message_add_to_queue(text_get_by_id(GAME_TEXT_ID_NOT_ENOUGH_GOLD + i),
								 NOT_ENOUGH_RESOURCE_TIME, PAL_COLOR_YELLOW, TRANSPARENT_INDEX);
			return;
		}
	}

	if (building_queue_training(building, unitType)) {
		// Deduct resources but not food
		for (int i = 0; i < UNIT_NORMAL_RESOURCES; i++) {
			resource_deduct_amount(context, building->controller, i, unitResources->used[i]);
		}
	} else {
		message_add_to_queue(text_get_by_id(GAME_TEXT_ID_QUEUE_FULL),
							 QUEUE_FULL_MSG_TIME, PAL_COLOR_YELLOW, TRANSPARENT_INDEX);
	}
}

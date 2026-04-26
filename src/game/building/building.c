#include "game/building/building.h"
#include <allegro.h>

#define QUEUE_FULL_MSG_TIME SEC_TO_FRAMES(5)
#define SPIRAL_DIRECTIONS 4
#define SPAWN_SHOW_TIME SEC_TO_FRAMES(3)

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

static uint8_t building_queue_training(GameUnit *building, TrainingTypeEnum trainingType) {
	BuildingData *buildingData = &building->typed.buildingData;
	if (buildingData->queueNextIndex >= MAX_BUILDING_QUEUE) return FALSE;
	buildingData->queue[buildingData->queueNextIndex++] = trainingType;
	return TRUE;
}

static void building_start_training(GameUnit *building, TrainingTypeEnum trainingType) {
	BuildingData *buildingData = &building->typed.buildingData;
	buildingData->isTraining = TRUE;
	buildingData->trainingType = trainingType;
	buildingData->currentTicks = 0;
	buildingData->targetTicks = game_unit_get_training_resources(trainingType)->time;
}

void building_update(GameContext *context, GameUnit *building) {
	BuildingData *buildingData = &building->typed.buildingData;
	if (buildingData->isTraining) {
		if (buildingData->currentTicks >= buildingData->targetTicks) {
			if (buildingData->trainingType >= TRAINING_TYPE_UPGRADE_SOLDIER) {
				// It's an upgrade, enable it and move on to the next in queue
				UnitTypeEnum unitType = buildingData->trainingType - TRAINING_TYPE_UPGRADE_SOLDIER + TRAINING_TYPE_SOLDIER;
				context->upgrades[building->controller][unitType].enabled = TRUE;
				buildingData->isTraining = FALSE;
			} else {
				if (resource_has_enough(context, building->controller, RESOURCE_TYPE_AVAILABLE_FOOD,
										game_unit_get_training_resources(buildingData->trainingType)->used[RESOURCE_TYPE_AVAILABLE_FOOD])) {
					Position spawnPosition = game_building_get_spawn_position(context, building);
					GameUnit *newUnit = game_unit_spawn(context, buildingData->trainingType, building->controller, spawnPosition.x, spawnPosition.y);
					if(newUnit) {
						buildingData->isTraining = FALSE;
						if(building->targetX != NO_TARGET_POSITION && building->targetY != NO_TARGET_POSITION) {
							game_unit_command_move(newUnit, NULL, building->targetX, building->targetY);
						}
					}
					else {
						buildingData->currentTicks = buildingData->targetTicks - 1;
					}
				} else {
					buildingData->currentTicks = buildingData->targetTicks - 1;
				}
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

static TrainingResourcesData* building_check_unit_resources(GameContext *context, ControllerEnum controller, TrainingTypeEnum trainingType) {
    TrainingResourcesData* unitResources = game_unit_get_training_resources(trainingType);
	for (int i = 0; i < UNIT_USED_RESOURCES; i++) {
		if (!resource_has_enough(context, controller, i, unitResources->used[i])) {
			if(controller == UNIT_CONTROLLER_PLAYER) {
				message_add_to_queue_shadow(text_get_by_id(GAME_TEXT_ID_NOT_ENOUGH_GOLD + i),
								 NOT_ENOUGH_RESOURCE_TIME, PAL_COLOR_YELLOW, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
				game_snd_play_sound(GAME_SOUND_NOT_VALID);
			}
			return NULL;
		}
	}
    return unitResources;
}

void building_add_to_train_queue(GameContext *context, GameUnit *building, TrainingTypeEnum trainingType) {
	// Check funds and food
    TrainingResourcesData* unitResources = building_check_unit_resources(context, building->controller, trainingType);
    if(!unitResources) return;

	if (building_queue_training(building, trainingType)) {
		// Deduct resources but not food
		for (int i = 0; i < UNIT_CREATE_REDUCE_RESOURCES; i++) {
			resource_deduct_amount(context, building->controller, i, unitResources->used[i]);
		}
	} else {
		if(building->controller == UNIT_CONTROLLER_PLAYER) {
			message_add_to_queue_shadow(text_get_by_id(GAME_TEXT_ID_QUEUE_FULL),
								QUEUE_FULL_MSG_TIME, PAL_COLOR_YELLOW, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
		}
	}
}

GameUnit* building_place_building(GameContext *context, UnitTypeEnum buildingType, ControllerEnum controller, uint16_t x, uint16_t y) {
	// Check funds and food
	TrainingResourcesData* unitResources = building_check_unit_resources(context, controller, buildingType);
	if(!unitResources) return NULL;

	GameUnit *building = game_unit_spawn(context, buildingType, controller, x, y);
	if (building) {
		if(building->controller == UNIT_CONTROLLER_PLAYER) game_snd_play_sound(GAME_SOUND_BUILDING_BUILD);
		// Deduct resources but not food
		for (int i = 0; i < UNIT_CREATE_REDUCE_RESOURCES; i++) {
			resource_deduct_amount(context, controller, i, unitResources->used[i]);
		}
	}
	return building;
}

void building_handle_placing_input(GameContext *context) {
    if(context->buildPlacing.state != CMD_BAR_BUILD_STATE_PLACE) return;

    if(context->mouseStatus.isRightPressed) {
        context->buildPlacing.state = CMD_BAR_BUILD_STATE_SELECT;
	} else {
		if (context->mouseStatus.isLeftPressed) {
			if (context->buildPlacing.canBuild) {
				GameUnit *building = building_place_building(context, context->buildPlacing.building,
															 UNIT_CONTROLLER_PLAYER,
															 context->buildPlacing.x, context->buildPlacing.y);
				if (building) {
					if (!keyboard_is_key_down(KEY_LSHIFT) && !keyboard_is_key_down(KEY_RSHIFT)) {
						context->buildPlacing.state = CMD_BAR_BUILD_STATE_NONE;
					}
					// Send selected worker to build it
					GameUnit *worker = game_unit_get_by_id(context, context->selectedUnits[0]);
					if (worker) {
						worker->typed.workerData.targetConstruction = building->id;
						game_unit_command_move(worker, building, NO_TARGET_POSITION, NO_TARGET_POSITION);
					}
				}
			} else {
				game_snd_play_sound(GAME_SOUND_NOT_VALID);
			}
		}
	}
}

void building_add_construction(GameContext *context, GameUnit *building) {
	BuildingData *buildingData = &building->typed.buildingData;
	buildingData->currentTicks += WORKER_TIME;

	uint32_t newAddedHealth = 0;
	if (buildingData->targetTicks > 0) {
		newAddedHealth = ((uint32_t) buildingData->currentTicks * building->maxHealth) / buildingData->targetTicks;
	}
	uint32_t healthInc = newAddedHealth - buildingData->addedHealth;
    buildingData->addedHealth = newAddedHealth;
	building->health += healthInc;
    if(building->health > building->maxHealth) building->health = building->maxHealth;

	if (building->typed.buildingData.currentTicks >= building->typed.buildingData.targetTicks) {
        building_complete(context, building);
	}
}

void building_repair(GameContext *context, GameUnit *building) {
    TrainingResourcesData* resources = game_unit_get_training_resources(building->type);
    uint16_t targetTicks = resources->time;
    uint32_t currentTicks = (((uint32_t) building->health * targetTicks) / building->maxHealth) + WORKER_TIME;
	building->health = (currentTicks * building->maxHealth) / targetTicks;
    if(building->health > building->maxHealth) building->health = building->maxHealth;
}

void building_complete(GameContext *context, GameUnit *building) {
    building->state = BUILDING_STATE_COMPLETED;
    game_animation_unit_set(building);
    // Provide food on completition
	TrainingResourcesData* trainingData = game_unit_get_training_resources(building->type);
    resource_add_food_provided(context, building->controller, trainingData->foodProvided);
    if(building->controller == UNIT_CONTROLLER_PLAYER) {
        message_add_to_queue_shadow(text_get_by_id(GAME_TEXT_ID_SPAWNED_WORKER + building->type),
            SPAWN_SHOW_TIME, PAL_COLOR_YELLOW, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
		game_unit_explore(context, building);
    }
}

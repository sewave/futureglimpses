#include "game/ai/strategy_ai.h"

// One frame checks attacks, one frame checks build options
#define ATTACK_WAVE_FRAMES SEC_TO_FRAMES(60) / 2
#define FIRST_WAVE_UNITS 4
#define MAX_WAVE_UNITS 16
#define SEARCH_RESOURCE_MULTIPLIER 5
#define MIN_GOLD_TRAINING_BUDGET 1000

static uint8_t trainRanged = FALSE;

static GameUnit* game_strategy_ai_get_computer_town_hall(GameContext *context) {
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_CITY_HALL) {
			return unit;
		}
	}
	return NULL;
}

static int game_strategy_ai_count_computer_workers(GameContext *context) {
	int count = 0;
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
			count++;
		}
	}
	// We also count workers in training and queued, to avoid overcommitting resources
	GameUnit *townHall = game_strategy_ai_get_computer_town_hall(context);
	if (townHall) {
		if (townHall->typed.buildingData.isTraining && townHall->typed.buildingData.trainUnit == UNIT_TYPE_WORKER) count++;
		for (int i = 0; i < townHall->typed.buildingData.queueNextIndex; i++) {
			if (townHall->typed.buildingData.queue[i] == UNIT_TYPE_WORKER) count++;
		}
	}
	return count;
}

static void game_strategy_ai_create_workers(GameContext *context) {
	if(game_strategy_ai_count_computer_workers(context) < context->aiData.desiredWorkers) {
		GameUnit *townHall = game_strategy_ai_get_computer_town_hall(context);
		// AI doesn't need queue, so add only if we are not training already,
		// this helps to save resources and distribute them better between creating workers
		// and other units
		if (townHall && !townHall->typed.buildingData.isTraining) {
			building_add_to_train_queue(context, townHall, UNIT_TYPE_WORKER);
		}
	}
}

static UnitPosition* game_strategy_ai_find_building_to_rebuild(GameContext *context) {
	for(int i = 0; i < context->aiData.initialBuildingsCount; i++) {
		UnitPosition *buildingPos = &context->aiData.initialBuildings[i];
		GameUnit *building = game_unit_get_by_id(context, context->walkabilityGrid[buildingPos->x][buildingPos->y]);
		if (!building || building->health < building->maxHealth / 2) {
			return buildingPos;
		}
	}
	return NULL;
}

static GameUnit* game_strategy_ai_find_building_to_repair(GameContext *context) {
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->isBuilding &&
			unit->health < unit->maxHealth && unit->state == BUILDING_STATE_COMPLETED) {
			return unit;
		}
	}
	return NULL;
}

static GameUnit* game_strategy_ai_find_building_to_construct(GameContext *context) {
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->isBuilding && unit->state != BUILDING_STATE_COMPLETED) {
			return unit;
		}
	}
	return NULL;
}

static GameUnit* game_strategy_ai_find_first_worker(GameContext *context) {
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
			return unit;
		}
	}
	return NULL;
}

static int game_strategy_ai_count_computer_workers_repairing(GameContext *context) {
	int count = 0;
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
			if (unit->typed.workerData.job == WORKER_JOB_REPAIR) count++;
		}
	}
	return count;
}

static void game_strategy_ai_builder_workers(GameContext *context) {
	GameUnit * builder = game_strategy_ai_find_first_worker(context);
	if (!builder) return;

	// We must clear builder workers that have completed their jobs
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
			WorkerData *workerData = &unit->typed.workerData;
			if (workerData->job == WORKER_JOB_REPAIR) {
				GameUnit *targetConstruction = game_unit_get_by_id(context, workerData->targetConstruction);
				if (!targetConstruction || targetConstruction->health >= targetConstruction->maxHealth) {
					workerData->targetConstruction = NO_TARGET_ID;
					workerData->job = WORKER_JOB_NONE;
					game_unit_command_idle(unit);
				}
			}
		}
	}
	
	UnitPosition *buildingToRebuild = game_strategy_ai_find_building_to_rebuild(context);
	if (buildingToRebuild) {
		int workerGoldCost = game_unit_get_resources(UNIT_TYPE_WORKER)->used[RESOURCE_TYPE_GOLD];
		int buildingGoldCost = game_unit_get_resources(buildingToRebuild->type)->used[RESOURCE_TYPE_GOLD];
		int gold = context->resources[UNIT_CONTROLLER_AI].uiQuantity[RESOURCE_TYPE_GOLD];
		// Reserve at least worker cost
		if (gold >= workerGoldCost + buildingGoldCost) {
			building_place_building(context, buildingToRebuild->type, UNIT_CONTROLLER_AI, buildingToRebuild->x, buildingToRebuild->y);
		}
	}

	GameUnit* buildingToWork = game_strategy_ai_find_building_to_construct(context);
	if(!buildingToWork) buildingToWork = game_strategy_ai_find_building_to_repair(context);

	if (buildingToWork) {
		// Send the first workers to work
		int assignedWorkers = game_strategy_ai_count_computer_workers_repairing(context);
		// Max 20% of workers repairing
		int maxWorkers = context->aiData.desiredWorkers / 5;
		if(maxWorkers < 1) maxWorkers = 1;
		GameUnit **activeList = context->activeUnits;
		for (int i = 0; i < context->activeUnitCount && assignedWorkers < maxWorkers; i++, activeList++) {
			GameUnit *unit = *activeList;
			if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
				WorkerData *workerData = &unit->typed.workerData;
				if (workerData->targetConstruction == NO_TARGET_ID) {
					workerData->targetConstruction = buildingToWork->id;
					workerData->job = WORKER_JOB_REPAIR;
					game_unit_command_move(unit, buildingToWork, NO_TARGET_POSITION, NO_TARGET_POSITION);
					assignedWorkers++;
				}
			}
		}
	}

}

static void game_strategy_ai_send_worker_to_harvest(GameContext *context, GameUnit *worker, WorkerJobEnum job) {
	Position resourcePos = {NO_TARGET_POSITION, NO_TARGET_POSITION};
	if (job == WORKER_JOB_GOLD) {
		resourcePos = resource_find_first_around_unit(
			context, worker, TILE_TYPE_GOLD, worker->sightRange * SEARCH_RESOURCE_MULTIPLIER);
	}
	else if (job == WORKER_JOB_WOOD) {
		resourcePos = resource_find_first_around_unit(
			context, worker, TILE_TYPE_WOOD, worker->sightRange * SEARCH_RESOURCE_MULTIPLIER);
	}
	if (resourcePos.x != NO_TARGET_POSITION && resourcePos.y != NO_TARGET_POSITION) {
		worker->typed.workerData.workplace = resourcePos;
		worker->typed.workerData.job = job;
		game_unit_command_move(worker, NULL, resourcePos.x, resourcePos.y);
	}
}

static void game_strategy_ai_harvester_workers(GameContext *context) {
	int currentWoodWorkers = 0, currentGoldWorkers = 0, workerCount = 0;
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
			WorkerData *workerData = &unit->typed.workerData;
			if (workerData->job == WORKER_JOB_WOOD) currentWoodWorkers++;
			else if (workerData->job == WORKER_JOB_GOLD) currentGoldWorkers++;
			if(workerData->job != WORKER_JOB_REPAIR) workerCount++;
		}
	}
	// 70 % gold, 30 % wood
	int desiredWoodWorkers = (workerCount * 30) / 100;
	int desiredGoldWorkers = workerCount - desiredWoodWorkers;
	int excessGoldWorkers = 0;
	if(desiredGoldWorkers < currentGoldWorkers) {
		excessGoldWorkers = currentGoldWorkers - desiredGoldWorkers;
	}
	int excessWoodWorkers = 0;
	if(desiredWoodWorkers < currentWoodWorkers) {
		excessWoodWorkers = currentWoodWorkers - desiredWoodWorkers;
	}
	int idleWorkersToGold = desiredGoldWorkers - currentGoldWorkers - excessWoodWorkers;
	if(idleWorkersToGold < 0) idleWorkersToGold = 0;
	int idleWorkersToWood = desiredWoodWorkers - currentWoodWorkers - excessGoldWorkers;
	if(idleWorkersToWood < 0) idleWorkersToWood = 0;
	// Change only needed workers, to avoid too much worker movement and loss of efficiency
	activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
			WorkerData *workerData = &unit->typed.workerData;
			if (workerData->job != WORKER_JOB_REPAIR) {
				if (workerData->job == WORKER_JOB_WOOD && excessWoodWorkers > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_GOLD);
					excessWoodWorkers--;
				}
				else if (workerData->job == WORKER_JOB_GOLD && excessGoldWorkers > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_WOOD);
					excessGoldWorkers--;
				}
				else if (workerData->job == WORKER_JOB_NONE && idleWorkersToGold > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_GOLD);
					idleWorkersToGold--;
				}
				else if (workerData->job == WORKER_JOB_NONE && idleWorkersToWood > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_WOOD);
					idleWorkersToWood--;
				}
			}
		}
	}
}

static void game_strategy_ai_train_units(GameContext *context) {
	uint8_t allWorkersTrained = game_strategy_ai_count_computer_workers(context) >= context->aiData.desiredWorkers;
	int workerGoldCost = game_unit_get_resources(UNIT_TYPE_WORKER)->used[RESOURCE_TYPE_GOLD];
	// Must have at least worker gold cost and reserve
	if(context->resources[UNIT_CONTROLLER_AI].uiQuantity[RESOURCE_TYPE_GOLD] <
		workerGoldCost + MIN_GOLD_TRAINING_BUDGET) return;
	uint8_t archersAvailable = context->map.enableBlacksmith && game_unit_exists(context, UNIT_CONTROLLER_AI, UNIT_TYPE_BLACKSMITH);
	uint8_t knightsAvailable = context->map.enableStables && game_unit_exists(context, UNIT_CONTROLLER_AI, UNIT_TYPE_STABLES);

	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->isBuilding &&
			unit->state == BUILDING_STATE_COMPLETED && !unit->typed.buildingData.isTraining) {
			int gold = context->resources[UNIT_CONTROLLER_AI].quantity[RESOURCE_TYPE_GOLD];
			if(unit->type == UNIT_TYPE_TOWER) {
				if (allWorkersTrained || gold >= workerGoldCost + game_unit_get_resources(UNIT_TYPE_MAGE)->used[RESOURCE_TYPE_GOLD]) {
					building_add_to_train_queue(context, unit, UNIT_TYPE_MAGE);
				}
			}
			if (unit->type == UNIT_TYPE_BARRACKS) {
				UnitTypeEnum typeToTrain = UNIT_TYPE_SOLDIER;
				if(trainRanged && archersAvailable) typeToTrain = UNIT_TYPE_ARCHER;
				if(!trainRanged && knightsAvailable) typeToTrain = UNIT_TYPE_KNIGHT;
				if (allWorkersTrained || gold >= workerGoldCost + game_unit_get_resources(typeToTrain)->used[RESOURCE_TYPE_GOLD]) {
					building_add_to_train_queue(context, unit, typeToTrain);
					trainRanged = !trainRanged;
				}
			}
		}
	}
}

static void game_strategy_ai_build_train(GameContext *context) {
	game_strategy_ai_create_workers(context);
	game_strategy_ai_builder_workers(context);
	game_strategy_ai_harvester_workers(context);
	game_strategy_ai_train_units(context);
}

static void game_strategy_ai_attack(GameContext *context) {
	if (++context->aiData.peaceCounter < context->map.peaceTime) return;
	if (++context->aiData.attackCounter < ATTACK_WAVE_FRAMES) return;
	context->aiData.attackCounter = 0;

	GameUnit *foundUnits[MAX_WAVE_UNITS];
	uint16_t foundUnitsCount = 0;
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && !unit->isBuilding && unit->type != UNIT_TYPE_WORKER) {
			foundUnits[foundUnitsCount++] = unit;
			if (foundUnitsCount == context->aiData.currentWaveUnits) break;
		}
	}
	if (foundUnitsCount != context->aiData.currentWaveUnits) return;

	// Now we search player CH, or if not found first active unit
	GameUnit *target = NULL;
	activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_PLAYER && unit->type == UNIT_TYPE_CITY_HALL) {
			target = unit;
			break;
		}
	}
	if (!target) {
		activeList = context->activeUnits;
		for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
			GameUnit *unit = *activeList;
			if (unit->isActive && unit->controller == UNIT_CONTROLLER_PLAYER) {
				target = unit;
				break;
			}
		}
	}
	if (target) {
		for (int i = 0; i < foundUnitsCount; i++) game_unit_command_move_attack(foundUnits[i], target, 0, 0);
		context->aiData.currentWaveUnits++;
	}
}

static void game_strategy_ai_scan_buildings(GameContext *context) {
	context->aiData.initialBuildingsCount = 0;
	GameUnit **activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->isBuilding) {
			context->aiData.initialBuildings[context->aiData.initialBuildingsCount++] = (UnitPosition){
					.type = unit->type,
					.x = unit->x,
					.y = unit->y,
			};
			if (context->aiData.initialBuildingsCount == MAX_AI_HANDLED_BUILDINGS) break;
		}
	}
}

void game_strategy_ai_init(GameContext *context) {
	context->aiData.attackCounter = 0;
	context->aiData.peaceCounter = 0;
	context->aiData.currentWaveUnits = FIRST_WAVE_UNITS;
	context->aiData.attackBuild = AI_BUILD;
	context->aiData.buildState = BUILD_STATE_CREATE_WORKERS;
	context->aiData.initialFood = context->resources[UNIT_CONTROLLER_AI].quantity[RESOURCE_TYPE_MAX_FOOD];
	context->aiData.desiredWorkers = context->aiData.initialFood / 4; // 25% must be workers
	game_strategy_ai_scan_buildings(context);
}

void game_strategy_ai_execute(GameContext *context) {
	if (context->aiData.attackBuild == AI_ATTACK) {
		context->aiData.attackBuild = AI_BUILD;
		// Cycle through build states, one frame will only do one action
		context->aiData.buildState = (context->aiData.buildState + 1) % BUILD_STATE_COUNT;
	} else {
		context->aiData.attackBuild = AI_ATTACK;
	}
	switch (context->map.aiMode) {
		case AI_MODE_IDLE: {
			// We do nothing :D
			break;
		}
		case AI_MODE_PASSIVE: {
			if (context->aiData.attackBuild == AI_BUILD) game_strategy_ai_build_train(context);
			break;
		}
		case AI_MODE_AGGRESSIVE: {
			if (context->aiData.attackBuild == AI_ATTACK) {
				game_strategy_ai_attack(context);
			} else {
				game_strategy_ai_build_train(context);
			}
			break;
		}
	}
}

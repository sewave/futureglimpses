#include "game/ai/strategy_ai.h"

// One frame checks attacks, one frame checks build options
#define ATTACK_WAVE_FRAMES SEC_TO_FRAMES(60) / 2
#define FIRST_WAVE_UNITS 4
#define MAX_WAVE_UNITS 16
#define CONSTRUCTION_WORKERS 4
#define SEARCH_RESOURCE_MULTIPLIER 5

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
		building_place_building(context, buildingToRebuild->type, UNIT_CONTROLLER_AI, buildingToRebuild->x, buildingToRebuild->y);
	}

	GameUnit* buildingToWork = game_strategy_ai_find_building_to_construct(context);
	if(!buildingToWork) buildingToWork = game_strategy_ai_find_building_to_repair(context);

	if (buildingToWork) {
		// Send the first workers to work
		int assignedWorkers = game_strategy_ai_count_computer_workers_repairing(context);
		GameUnit **activeList = context->activeUnits;
		for (int i = 0; i < context->activeUnitCount && assignedWorkers < CONSTRUCTION_WORKERS; i++, activeList++) {
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
	int desiredWoodWorkers = workerCount / 2;
	int desiredGoldWorkers = workerCount - desiredWoodWorkers;
	int goldToWoodWorkersToChange = abs(desiredWoodWorkers - currentWoodWorkers);
	int woodToGoldWorkersToChange = abs(desiredGoldWorkers - currentGoldWorkers);
	int otherToGoldWorkersToChange = desiredGoldWorkers - currentGoldWorkers - goldToWoodWorkersToChange;
	int otherToWoodWorkersToChange = desiredWoodWorkers - currentWoodWorkers - woodToGoldWorkersToChange;
	// Change only needed workers, to avoid too much worker movement and loss of efficiency
	activeList = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
		GameUnit *unit = *activeList;
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI && unit->type == UNIT_TYPE_WORKER) {
			WorkerData *workerData = &unit->typed.workerData;
			if (workerData->job != WORKER_JOB_REPAIR) {
				if (workerData->job == WORKER_JOB_WOOD && woodToGoldWorkersToChange > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_GOLD);
					woodToGoldWorkersToChange--;
				}
				else if (workerData->job == WORKER_JOB_GOLD && goldToWoodWorkersToChange > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_WOOD);
					goldToWoodWorkersToChange--;
				}
				else if (workerData->job == WORKER_JOB_NONE && otherToGoldWorkersToChange > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_GOLD);
					otherToGoldWorkersToChange--;
				}
				else if (workerData->job == WORKER_JOB_NONE && otherToWoodWorkersToChange > 0) {
					game_strategy_ai_send_worker_to_harvest(context, unit, WORKER_JOB_WOOD);
					otherToWoodWorkersToChange--;
				}
			}
		}
	}
}

static void game_strategy_ai_train_units(GameContext *context) {
	// TODO Create units on barracks/tower, only if at least there is money for a worker or all desired workers are trained
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
		if (unit->isActive && unit->controller == UNIT_CONTROLLER_AI) {
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

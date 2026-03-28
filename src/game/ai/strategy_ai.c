#include "game/ai/strategy_ai.h"

// One frame checks attacks, one frame checks build options
#define ATTACK_WAVE_FRAMES SEC_TO_FRAMES(60) / 2
#define FIRST_WAVE_UNITS 4
#define MAX_WAVE_UNITS 16

static void game_strategy_ai_create_workers(GameContext *context) {
	// TODO: Have 25% of initial food on workers (Fixed by map)
}

static void game_strategy_ai_builder_workers(GameContext *context) {
	// TODO: Have 2-4 workers repair/reconstruct buildings (can steal from other tasks)
}

static void game_strategy_ai_harvester_workers(GameContext *context) {
	// TODO: Have remaining workers with assigned tasks 50/50 gold/wood
}

static void game_strategy_ai_train_units(GameContext *context) {
	// TODO: Create units on barracks/tower, only if at least there is money for a worker or all desired workers are trained
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

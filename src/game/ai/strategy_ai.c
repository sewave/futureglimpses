#include "game/ai/strategy_ai.h"

#define ATTACK_WAVE_FRAMES SEC_TO_FRAMES(60)
#define FIRST_WAVE_UNITS 4
#define MAX_WAVE_UNITS 32

static void game_strategy_ai_build_train(GameContext *context) {
	// TODO, everything except send attacks
	// Map must have all required buildings, so we train, harvest, repair and rebuild
	// Training goes:
	// 1) Have 10% of initial food on workers (Fixed by map)
	// 2) Have 1-2 workers repair/construct buildings (can steal from other tasks)
	// 3) Have remaining workers with assigned tasks 50/50 gold/wood
	// 4) Rebuild destroyed buildings (check list), if there is enemy here, attack, else build (if has the resources)
	// 5) Train units (and send them somewhere?)
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
		for (int i = 0; i < foundUnitsCount; i++) {
			GameUnit* unit = foundUnits[i];
			game_unit_command_move_attack(unit, target, 0, 0);
		}
		context->aiData.currentWaveUnits++;
	}
}

void game_strategy_ai_init(GameContext *context) {
	context->aiData.attackCounter = 0;
    context->aiData.peaceCounter = 0;
	context->aiData.currentWaveUnits = FIRST_WAVE_UNITS;
	// TODO init all ai variables
	// TODO scan buildings to track rebuild later
}

void game_strategy_ai_execute(GameContext *context) {
	switch (context->map.aiMode) {
		case AI_MODE_IDLE: {
			// We do nothing :D
			break;
		}
		case AI_MODE_PASSIVE: {
			game_strategy_ai_build_train(context);
			break;
		}
		case AI_MODE_AGGRESSIVE: {
			game_strategy_ai_build_train(context);
			game_strategy_ai_attack(context);
			break;
		}
	}
}

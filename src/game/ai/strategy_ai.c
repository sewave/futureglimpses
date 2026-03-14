#include "game/ai/strategy_ai.h"

#define ATTACK_WAVE_FRAMES SEC_TO_FRAMES(60)
#define FIRST_WAVE_UNITS 4
#define MAX_WAVE_UNITS 32

static const uint16_t quadrantXPositions[3] = { BOARD_WIDTH / 6, BOARD_WIDTH / 2, (BOARD_WIDTH * 5) / 6 };
static const uint16_t quadrantYPositions[3] = { BOARD_HEIGHT / 6, BOARD_HEIGHT / 2, (BOARD_HEIGHT * 5) / 6 };

static uint8_t game_spatial_filter_idle_ai_units(const GameContext* context, const GameUnit *source, const GameUnit *found) {
	return found->isActive && found->controller == UNIT_CONTROLLER_AI && found->state == UNIT_STATE_IDLE;
}

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

    // TODO search for wave units, return if not enought

	// We will spatial query select up to 10 idle units from a quadrant and send them to attack another one
	uint16_t sourceX = quadrantXPositions[random_int(0, 2)];
	uint16_t sourceY = quadrantYPositions[random_int(0, 2)];
    uint16_t unitsSent = 0;

    GameUnit* foundUnits[MAX_WAVE_UNITS];

    uint16_t foundUnitsCount = game_spatial_query_grid_rectangle(context,
        sourceX - BOARD_WIDTH / 6, sourceY - BOARD_HEIGHT / 6,
        sourceX + BOARD_WIDTH / 6, sourceY + BOARD_HEIGHT / 6,
        game_spatial_filter_idle_ai_units,
        NULL,
        foundUnits, 10
    );

    uint16_t targetX = quadrantXPositions[random_int(0, 2)];
    uint16_t targetY = quadrantYPositions[random_int(0, 2)];
    for (uint16_t i = 0; i < foundUnitsCount; i++) {
        GameUnit* unit = foundUnits[i];
        game_unit_command_move_attack(unit, NULL, targetX, targetY);
        unitsSent++;
    }
    context->aiData.currentWaveUnits++;
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

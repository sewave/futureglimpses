#include "game/ai/strategy_ai.h"

static uint16_t strategyCounter = 0;
static const uint16_t quadrantXPositions[3] = { BOARD_WIDTH / 6, BOARD_WIDTH / 2, (BOARD_WIDTH * 5) / 6 };
static const uint16_t quadrantYPositions[3] = { BOARD_HEIGHT / 6, BOARD_HEIGHT / 2, (BOARD_HEIGHT * 5) / 6 };

static uint8_t game_spatial_filter_idle_ai_units(const GameContext* context, const GameUnit *source, const GameUnit *found) {
	return found->isActive && found->controller == UNIT_CONTROLLER_AI && found->state == UNIT_STATE_IDLE;
}

void game_strategy_ai_execute(GameContext* context) {
    // For now, we will select groups of up to 10 idle units and send them to attack a random position in the map
    // this position will be one of 9 possible quadrants in the map
    if (++strategyCounter < SEC_TO_FRAMES(3)) return;
    strategyCounter = 0;
    // We will spatial query select up to 10 idle units from a quadrant and send them to attack another one

    uint16_t sourceX = quadrantXPositions[random_int(0, 2)];
    uint16_t sourceY = quadrantYPositions[random_int(0, 2)];
    uint16_t unitsSent = 0;

    GameUnit* foundUnits[10];

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
}

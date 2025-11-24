#include "unit.h"

GameUnit* game_unit_get_by_handle(GameState* gameState, int handle) {
    int index = GET_INDEX(handle);
    int gen = GET_GEN(handle);

    if (index < 0 || index >= MAX_GAME_UNITS) return NULL;
    if (!gameState->units[index].active) return NULL;
    if (gameState->unitGenerations[index] != gen) return NULL;

    return &gameState->units[index];
}

void game_unit_destroy(GameState* gameState, int handle) {
    GameUnit* u = game_unit_get_by_handle(gameState, handle);
    if (u) u->active = FALSE;
}

void game_units_init(GameState* gameState) {
    for (int i = 0; i < MAX_GAME_UNITS; i++) {
        gameState->units[i].active = FALSE;
        gameState->unitGenerations[i] = FIRST_UNIT_GENERATION;
    }
}

int game_unit_spawn(GameState* gameState, GameUnit* unitData) {
    int index = HANDLER_NOT_FOUND;
    for (int i = 0; i < MAX_GAME_UNITS; i++) {
        if (!gameState->units[i].active) {
            index = i;
            break;
        }
    }

    if (index == HANDLER_NOT_FOUND) {
        printf("Error: Max units reached!\n");
        return HANDLER_NOT_FOUND;
    }

    gameState->unitGenerations[index]++;

    GameUnit* unit = &gameState->units[index];
    memcpy(unit, unitData, sizeof(GameUnit));
    unit->id = MAKE_ID(index, gameState->unitGenerations[index]);
    unit->active = TRUE;

    return unit->id;
}

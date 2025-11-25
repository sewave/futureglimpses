#include "unit.h"

#define FIRST_UNIT_GENERATION 1
static unsigned short unitGenerations[MAX_GAME_UNITS];

GameUnit *game_unit_get_by_handle(GameContext *gameContext, int handle) {
	int index = GET_INDEX(handle);
	int gen = GET_GEN(handle);

	if (index < 0 || index >= MAX_GAME_UNITS) return NULL;
	if (!gameContext->units[index].active) return NULL;
	if (unitGenerations[index] != gen) return NULL;

	return &gameContext->units[index];
}

void game_unit_destroy(GameContext *gameContext, int handle) {
	GameUnit *u = game_unit_get_by_handle(gameContext, handle);
	if (u) u->active = FALSE;
}

void game_units_init(GameContext *gameContext) {
	for (int i = 0; i < MAX_GAME_UNITS; i++) {
		gameContext->units[i].active = FALSE;
		unitGenerations[i] = FIRST_UNIT_GENERATION;
	}
}

int game_unit_spawn(GameContext *gameContext, GameUnit *unitData) {
	int index = FREE_UNIT_SLOT_NOT_FOUND;
	for (int i = 0; i < MAX_GAME_UNITS; i++) {
		if (!gameContext->units[i].active) {
			index = i;
			break;
		}
	}

	if (index == FREE_UNIT_SLOT_NOT_FOUND) {
		printf("Error: Max units reached!\n");
		return FREE_UNIT_SLOT_NOT_FOUND;
	}

	unitGenerations[index]++;

	GameUnit *unit = &gameContext->units[index];
	memcpy(unit, unitData, sizeof(GameUnit));
	unit->id = MAKE_ID(index, unitGenerations[index]);
	unit->active = TRUE;

	return unit->id;
}

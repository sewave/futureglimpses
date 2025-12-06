#include "unit.h"

#define FIRST_UNIT_GENERATION 1
#define NO_FREE_UNIT_INDEX -1
static unsigned short unitGenerations[MAX_GAME_UNITS];
static int activeIndices[MAX_GAME_UNITS];
static int activeUnitCount;
static int nextFreeIndex;
static int selectedUnits[MAX_GAME_UNITS];
static int selectedUnitCount = 0;

static int game_unit_find_free_index(GameUnit units[]) {
	int startIndex = nextFreeIndex;
	int index = NO_FREE_UNIT_INDEX;

	for (int i = 0; i < MAX_GAME_UNITS; i++) {
		int checkIndex = (startIndex + i) % MAX_GAME_UNITS;
		if (!units[checkIndex].isActive) {
			index = checkIndex;
			nextFreeIndex = (index + 1) % MAX_GAME_UNITS;
			break;
		}
	}

	return index;
}

void game_units_init(GameContext *context) {
	for (int i = 0; i < MAX_GAME_UNITS; i++) {
		context->units[i].isActive = FALSE;
		context->units[i].id = NULL_HANDLE;
		unitGenerations[i] = FIRST_UNIT_GENERATION;
	}
	activeUnitCount = 0;
	nextFreeIndex = 0;
}

GameUnit *game_unit_get_by_id(GameContext *context, UnitId id) {
	int index = GET_INDEX(id);
	int gen = GET_GEN(id);

	if (index < 0 || index >= MAX_GAME_UNITS) return NULL;
	if (!context->units[index].isActive) return NULL;
	if (unitGenerations[index] != gen) return NULL;

	return &context->units[index];
}

void game_unit_destroy(GameContext *context, UnitId id) {
	GameUnit *unit = game_unit_get_by_id(context, id);
	if (unit && unit->isActive) {
		unit->isActive = FALSE;
		context->walkabilityGrid[unit->x][unit->y] = WALKABILITY_FREE;
	}
}

GameUnit *game_unit_spawn(GameContext *context, GameUnit *unitToSpawn) {
	if (context->walkabilityGrid[unitToSpawn->x][unitToSpawn->y] != WALKABILITY_FREE) return NULL;
	int index = game_unit_find_free_index(context->units);
	if (index == NO_FREE_UNIT_INDEX) return NULL;
	GameUnit *unit = &context->units[index];
	unitGenerations[index]++;
	memcpy(unit, unitToSpawn, sizeof(GameUnit));
	unit->id = MAKE_ID(index, unitGenerations[index]);
	unit->isActive = TRUE;
	game_animation_unit_set(unit);
	game_gfx_set_sprite_sheet(unit);
	// Register unit in walkability
	context->walkabilityGrid[unit->x][unit->y] = unit->id;
	return unit;
}

void game_unit_face_target(GameUnit *unit, GameUnit *target) {
	// If we are not moving, we face our enemy
	if (unit->state != UNIT_STATE_MOVE_ANIM) {
		if (target->y < unit->y) {
			unit->direction = DIRECTION_NORTH;
		} else {
			if (target->y > unit->y) {
				unit->direction = DIRECTION_SOUTH;
			} else {
				if (target->x < unit->x) {
					unit->direction = DIRECTION_EAST;
				} else {
					if (target->x > unit->y) {
						unit->direction = DIRECTION_WEST;
					}
				}
			}
		}
	}
}

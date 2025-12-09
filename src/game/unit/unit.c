#include "unit.h"

#define FIRST_UNIT_GENERATION 1
#define NO_FREE_UNIT_INDEX -1
static unsigned short unitGenerations[MAX_GAME_UNITS];
static uint16_t nextFreeIndex;

typedef struct {
	UnitTypeEnum type;
	uint8_t isBuilding;
	uint8_t attackRange, sightRange;
	uint16_t health, maxHealth;
	uint8_t tileSize;
	uint8_t minDamage;
	uint8_t maxDamage;
	uint16_t reactionTime;
	uint16_t moveTime;
} UnitData;

UnitData unitsData[UNIT_TYPE_NUMBER] = {
	{ 
		.type = UNIT_TYPE_WORKER,
		.isBuilding = FALSE,
		.attackRange = 1,
		.sightRange = 5,
		.health = 50,
		.maxHealth = 50,
		.tileSize = 1,
		.minDamage = 2,
		.maxDamage = 4,
		.reactionTime = SEC_TO_FRAMES(1),
		.moveTime = SEC_TO_FRAMES(0.5),
	},
	{
		.type = UNIT_TYPE_SOLDIER,
		.isBuilding = FALSE,
		.attackRange = 1,
		.sightRange = 6,
		.health = 100,
		.maxHealth = 100,
		.tileSize = 1,
		.minDamage = 8,
		.maxDamage = 12,
		.reactionTime = SEC_TO_FRAMES(0.5),
		.moveTime = SEC_TO_FRAMES(0.4),
	},
	{
		.type = UNIT_TYPE_ARCHER,
		.isBuilding = FALSE,
		.attackRange = 5,
		.sightRange = 7,
		.health = 75,
		.maxHealth = 75,
		.tileSize = 1,
		.minDamage = 6,
		.maxDamage = 10,
		.reactionTime = SEC_TO_FRAMES(0.7),
		.moveTime = SEC_TO_FRAMES(0.45),
	},
	{
		.type = UNIT_TYPE_MOUNT,
		.isBuilding = FALSE,
		.attackRange = 1,
		.sightRange = 6,
		.health = 120,
		.maxHealth = 120,
		.tileSize = 1,
		.minDamage = 10,
		.maxDamage = 15,
		.reactionTime = SEC_TO_FRAMES(0.4),
		.moveTime = SEC_TO_FRAMES(0.3),
	},
	{
		.type = UNIT_TYPE_MAGE,
		.isBuilding = FALSE,
		.attackRange = 4,
		.sightRange = 6,
		.health = 80,
		.maxHealth = 80,
		.tileSize = 1,
		.minDamage = 12,
		.maxDamage = 18,
		.reactionTime = SEC_TO_FRAMES(0.8),
		.moveTime = SEC_TO_FRAMES(0.5),
	},
	{},
	{},
	{},
	{},
	{},
	{},
};

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
	context->activeUnitCount = 0;
	context->selectedUnitCount = 0;
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
		GameUnit **activeList = context->activeUnits;
		for (int i = 0; i < context->activeUnitCount; i++, activeList++) {
			if (*activeList == unit) {
				*activeList = context->activeUnits[--context->activeUnitCount];
				break;
			}
		}
		if (unit->isSelected) {
			for (int i = 0; i < context->selectedUnitCount; i++) {
				if (context->selectedUnits[i] == id) {
					context->selectedUnits[i] = context->selectedUnits[--context->selectedUnitCount];
					break;
				}
			}
			unit->isSelected = FALSE;
		}
	}
}

GameUnit *game_unit_spawn(GameContext *context, UnitTypeEnum type, UnitControllerEnum controller, uint16_t x, uint16_t y) {
	if (context->walkabilityGrid[x][y] != WALKABILITY_FREE) return NULL;
	int index = game_unit_find_free_index(context->units);
	if (index == NO_FREE_UNIT_INDEX) return NULL;
	GameUnit *unit = &context->units[index];
	unitGenerations[index]++;
	unit->id = MAKE_ID(index, unitGenerations[index]);
	unit->controller = controller;
	unit->x = x;
	unit->y = y;
	unit->prevX = unit->x;
	unit->prevY = unit->y;
	unit->state = UNIT_STATE_IDLE;
	unit->nextState = UNIT_STATE_IDLE;
	unit->direction = DIRECTION_SOUTH;
	unit->reactionTimeCounter = 0;
	unit->moveTimeCounter = 0;
	unit->isActive = TRUE;
	unit->isSelected = FALSE;
	unit->targetX = NO_TARGET_POSITION;
	unit->targetY = NO_TARGET_POSITION;
	unit->targetId = NO_TARGET_ID;
	unit->blinkTime = 0;
	
	UnitData* data = &unitsData[type];
	unit->type = data->type;
	unit->isBuilding = data->isBuilding;
	unit->attackRange = data->attackRange;
	unit->sightRange = data->sightRange;
	unit->health = data->health;
	unit->maxHealth = data->maxHealth;
	unit->tileSize = data->tileSize;
	unit->minDamage = data->minDamage;
	unit->maxDamage = data->maxDamage;
	unit->reactionTime = data->reactionTime;
	unit->moveTime = data->moveTime;

	game_animation_unit_set(unit);
	game_gfx_set_sprite_sheet(unit);
	// Register unit in walkability
	// TODO handle larger units
	context->walkabilityGrid[unit->x][unit->y] = unit->id;
	// Add to active list
	context->activeUnits[context->activeUnitCount++] = unit;
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
					unit->direction = DIRECTION_WEST;
				} else {
					if (target->x > unit->x) {
						unit->direction = DIRECTION_EAST;
					}
				}
			}
		}
	}
}

void game_unit_damage(GameContext* context, GameUnit* unit, GameUnit* target) {
	if(!unit->isActive || !target->isActive || target->state == UNIT_STATE_DIE) return;
	uint8_t damage = random_int(unit->minDamage, unit->maxDamage);
	if(target->health <= damage) {
		target->health = 0;
		target->state = UNIT_STATE_DIE;
		game_animation_unit_set(target);
	}
	else {
		target->health -= damage;
	}
}

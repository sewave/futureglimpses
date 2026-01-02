#include "unit.h"

#define FIRST_UNIT_GENERATION 1
#define NO_FREE_UNIT_INDEX -1
#define AREA_DAMAGE_REDUCTION 4
#define SPAWN_SHOW_TIME SEC_TO_FRAMES(3)
static unsigned short unitGenerations[MAX_GAME_UNITS];
static uint16_t nextFreeIndex;

static UnitData unitsData[UNIT_TYPE_NUMBER] = {
		{
				.type = UNIT_TYPE_WORKER,
				.isBuilding = FALSE,
				.minAttackRange = 0,
				.maxAttackRange = 1,
				.sightRange = 5,
				.health = 50,
				.maxHealth = 50,
				.tileSize = 1,
				.minDamage = 2,
				.maxDamage = 4,
				.reactionTime = SEC_TO_FRAMES(1),
				.moveTime = SEC_TO_FRAMES(0.5),
				{.used = {400, 0, 1}, .time = SEC_TO_FRAMES(15), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_SOLDIER,
				.isBuilding = FALSE,
				.minAttackRange = 0,
				.maxAttackRange = 1,
				.sightRange = 6,
				.health = 100,
				.maxHealth = 100,
				.tileSize = 1,
				.minDamage = 8,
				.maxDamage = 12,
				.reactionTime = SEC_TO_FRAMES(0.5),
				.moveTime = SEC_TO_FRAMES(0.4),
				{.used = {600, 0, 1}, .time = SEC_TO_FRAMES(20), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_ARCHER,
				.isBuilding = FALSE,
				.minAttackRange = 1,
				.maxAttackRange = 5,
				.sightRange = 7,
				.health = 75,
				.maxHealth = 75,
				.tileSize = 1,
				.minDamage = 6,
				.maxDamage = 10,
				.reactionTime = SEC_TO_FRAMES(0.7),
				.moveTime = SEC_TO_FRAMES(0.45),
				{.used = {500, 50, 1}, .time = SEC_TO_FRAMES(25), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_KNIGHT,
				.isBuilding = FALSE,
				.minAttackRange = 0,
				.maxAttackRange = 1,
				.sightRange = 6,
				.health = 120,
				.maxHealth = 120,
				.tileSize = 1,
				.minDamage = 10,
				.maxDamage = 15,
				.reactionTime = SEC_TO_FRAMES(0.4),
				.moveTime = SEC_TO_FRAMES(0.3),
				{.used = {800, 100, 1}, .time = SEC_TO_FRAMES(30), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_MAGE,
				.isBuilding = FALSE,
				.minAttackRange = 2,
				.maxAttackRange = 8,
				.sightRange = 10,
				.health = 80,
				.maxHealth = 80,
				.tileSize = 1,
				.minDamage = 10,
				.maxDamage = 15,
				.reactionTime = SEC_TO_FRAMES(0.8),
				.moveTime = SEC_TO_FRAMES(0.8),
				{.used = {900, 300, 1}, .time = SEC_TO_FRAMES(83), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_CITY_HALL,
				.isBuilding = TRUE,
				.minAttackRange = 0,
				.maxAttackRange = 0,
				.sightRange = 10,
				.health = 1000,
				.maxHealth = 1000,
				.tileSize = 3,
				.minDamage = 0,
				.maxDamage = 0,
				.reactionTime = 0,
				.moveTime = 0,
				{.used = {1000, 0, 0}, .time = SEC_TO_FRAMES(85), .foodProvided = 5},
		},
		{
				.type = UNIT_TYPE_FARM,
				.isBuilding = TRUE,
				.minAttackRange = 0,
				.maxAttackRange = 0,
				.sightRange = 5,
				.health = 500,
				.maxHealth = 500,
				.tileSize = 2,
				.minDamage = 0,
				.maxDamage = 0,
				.reactionTime = 0,
				.moveTime = 0,
				{.used = {500, 250, 0}, .time = SEC_TO_FRAMES(33), .foodProvided = 4},
		},
		{
				.type = UNIT_TYPE_BARRACKS,
				.isBuilding = TRUE,
				.minAttackRange = 0,
				.maxAttackRange = 0,
				.sightRange = 8,
				.health = 800,
				.maxHealth = 800,
				.tileSize = 3,
				.minDamage = 0,
				.maxDamage = 0,
				.reactionTime = 0,
				.moveTime = 0,
				{.used = {700, 450, 0}, .time = SEC_TO_FRAMES(67), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_BLACKSMITH,
				.isBuilding = TRUE,
				.minAttackRange = 0,
				.maxAttackRange = 0,
				.sightRange = 6,
				.health = 650,
				.maxHealth = 650,
				.tileSize = 2,
				.minDamage = 0,
				.maxDamage = 0,
				.reactionTime = 0,
				.moveTime = 0,
				{.used = {800, 450, 0}, .time = SEC_TO_FRAMES(67), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_STABLES,
				.isBuilding = TRUE,
				.minAttackRange = 0,
				.maxAttackRange = 0,
				.sightRange = 7,
				.health = 850,
				.maxHealth = 850,
				.tileSize = 3,
				.minDamage = 0,
				.maxDamage = 0,
				.reactionTime = 0,
				.moveTime = 0,
				{.used = {1000, 300, 0}, .time = SEC_TO_FRAMES(50), .foodProvided = 0},
		},
		{
				.type = UNIT_TYPE_TOWER,
				.isBuilding = TRUE,
				.minAttackRange = 0,
				.maxAttackRange = 0,
				.sightRange = 6,
				.health = 750,
				.maxHealth = 750,
				.tileSize = 2,
				.minDamage = 0,
				.maxDamage = 0,
				.reactionTime = 0,
				.moveTime = 0,
				{.used = {1000, 200, 0}, .time = SEC_TO_FRAMES(42), .foodProvided = 0},
		},
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
		for(int i = unit->x; i < unit->x + unit->tileSize; i++) {
			for(int j = unit->y; j < unit->y + unit->tileSize; j++) {
				context->walkabilityGrid[i][j] = WALKABILITY_FREE;
			}
		}
		
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
		uint8_t opponentController = opponent_controller(unit->controller);
		if (unit->isBuilding) {
			context->stats[opponentController].buildingsDestroyed++;
		} else {
			context->stats[opponentController].enemiesKilled++;
		}
		UnitData *data = &unitsData[unit->type];
		resource_deduct_food(context, unit->controller, data->resources.used[RESOURCE_TYPE_AVAILABLE_FOOD], data->resources.foodProvided);
	}
}

GameUnit *game_unit_spawn(GameContext *context, UnitTypeEnum type, ControllerEnum controller, uint16_t x, uint16_t y) {
	UnitData* unitData = &unitsData[type];
	for(int i = x; i < x + unitData->tileSize; i++) {
		for(int j = y; j < y + unitData->tileSize; j++) {
			if(i > BOARD_X_MAX || j > BOARD_Y_MAX) return NULL;
			if (context->walkabilityGrid[i][j] != WALKABILITY_FREE) return NULL;
		}
	}
	
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
	unit->direction = DIRECTION_SOUTH;
	unit->reactionTimeCounter = 0;
	unit->moveTimeCounter = 0;
	unit->isActive = TRUE;
	unit->isSelected = FALSE;
	unit->targetX = NO_TARGET_POSITION;
	unit->targetY = NO_TARGET_POSITION;
	unit->prevTargetX = NO_TARGET_POSITION;
	unit->prevTargetY = NO_TARGET_POSITION;	
	unit->targetId = NO_TARGET_ID;
	unit->prevTargetId = NO_TARGET_ID;
	unit->blinkTime = 0;

	UnitData *data = &unitsData[type];
	unit->type = data->type;
	unit->isBuilding = data->isBuilding;
	unit->minAttackRange = data->minAttackRange;
	unit->maxAttackRange = data->maxAttackRange;
	unit->sightRange = data->sightRange;
	unit->maxHealth = data->maxHealth;
	unit->tileSize = data->tileSize;
	unit->minDamage = data->minDamage;
	unit->maxDamage = data->maxDamage;
	unit->reactionTime = data->reactionTime;
	unit->moveTime = data->moveTime;

	if(unit->type == UNIT_TYPE_WORKER) {
		unit->typed.workerData.targetConstruction = NO_TARGET_ID;
	}

	if(unit->isBuilding) {
		unit->health = 1;
		unit->state = BUILDING_STATE_CONSTRUCT;
		unit->nextState = BUILDING_STATE_CONSTRUCT;

		BuildingData *buildingData = &unit->typed.buildingData;
		buildingData->isTraining = FALSE;
		buildingData->queueNextIndex = 0;
		buildingData->addedHealth = unit->health;
		buildingData->currentTicks = 0;
		buildingData->targetTicks = data->resources.time;
	}
	else {
		unit->health = data->health;
		unit->state = UNIT_STATE_IDLE;
		unit->nextState = UNIT_STATE_IDLE;
	}

	resource_add_food_usage(context, unit->controller, data->resources.used[RESOURCE_TYPE_AVAILABLE_FOOD]);

	game_animation_unit_set(unit);
	game_gfx_set_sprite_sheet(unit);
	// Register unit in walkability
	for(int i = unit->x; i < unit->x + unit->tileSize; i++) {
		for(int j = unit->y; j < unit->y + unit->tileSize; j++) {
			context->walkabilityGrid[i][j] = unit->id;	
		}
	}
	
	// Add to active list
	context->activeUnits[context->activeUnitCount++] = unit;
	// Add spawn to stats
	if (unit->isBuilding) {
		context->stats[controller].buildingsConstructed++;
	} else {
		context->stats[controller].unitsTrained++;
	}

	if(unit->controller == UNIT_CONTROLLER_PLAYER && !unit->isBuilding) {
		message_add_to_queue(text_get_by_id(GAME_TEXT_ID_SPAWNED_WORKER + unit->type),
		SPAWN_SHOW_TIME, PAL_COLOR_YELLOW, TRANSPARENT_INDEX);
	}

	// TODO spawn sound? building placed / unit ready

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

void game_unit_damage(GameContext *context, uint8_t minDamage, uint8_t maxDamage, GameUnit *target) {
	if (!target->isActive || target->state == UNIT_STATE_DIE) return;
	uint8_t damage = random_int(minDamage, maxDamage);
	if (target->health <= damage) {
		target->health = 0;
		target->state = UNIT_STATE_DIE;
		game_animation_unit_set(target);
	} else {
		target->health -= damage;
	}
}

void game_unit_area_damage(GameContext *context, Object *object) {
	uint16_t centerX = object->targetX / TILE_SIZE;
	uint16_t centerY = object->targetY / TILE_SIZE;
	for (int row = centerY - object->damageRadius; row <= centerY + object->damageRadius; row++) {
		for (int col = centerX - object->damageRadius; col <= centerX + object->damageRadius; col++) {
			if (col < BOARD_X_MIN || col > BOARD_X_MAX || row < BOARD_Y_MIN || row > BOARD_Y_MAX) continue;
			UnitId id = context->walkabilityGrid[col][row];
			if (id < HANDLE_ID_THRESHOLD) continue;
			GameUnit *target = game_unit_get_by_id(context, id);
			if (target) {
				if(col == centerX || row == centerY) {
					game_unit_damage(context, object->minDamage, object->maxDamage, target);
				}
				else {
					game_unit_damage(context, object->minDamage / AREA_DAMAGE_REDUCTION, object->maxDamage / AREA_DAMAGE_REDUCTION, target);
				}
				GameUnit *sourceUnit = game_unit_get_by_id(context, object->ownerId);
				if (target->state == UNIT_STATE_IDLE && sourceUnit
					&& sourceUnit->controller != target->controller) {
					game_unit_command_move_attack(target, sourceUnit, sourceUnit->x, sourceUnit->y);
				}
			}
		}
	}
}

void game_unit_process_all(GameContext *context) {
	// TODO performance - not updating minimap each frame?
	// Minimap update
	clear_bitmap(context->renderedMinimapUnits);
	GameUnit **activeUnits = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeUnits++) {
		GameUnit *unit = *activeUnits;
		game_animation_unit_advance(context, unit);
		game_unit_ai_invoke(context, unit);
		int color = unit->controller == UNIT_CONTROLLER_PLAYER ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		if (unit->tileSize == 1) {
			putpixel(context->renderedMinimapUnits, unit->x, unit->y, color);
		} else {
			for (int dx = 0; dx < unit->tileSize; dx++) {
				for (int dy = 0; dy < unit->tileSize; dy++) {
					putpixel(context->renderedMinimapUnits, unit->x + dx, unit->y + dy, color);
				}
			}
		}
	}
}

UnitResourcesData* game_unit_get_resources(UnitTypeEnum type) {
	return &unitsData[type].resources;
}

UnitData* game_unit_get_data(UnitTypeEnum type) {
	return &unitsData[type];
}

void game_unit_work(GameContext *context, GameUnit *worker) {
	if (worker->type != UNIT_TYPE_WORKER) return;
	GameUnit *workTarget = game_unit_get_by_id(context, worker->typed.workerData.targetConstruction);
	if (workTarget && workTarget->isBuilding) {
		if(workTarget->state == BUILDING_STATE_COMPLETED) {
			if(workTarget->health < workTarget->maxHealth) {
				building_repair(context, workTarget);
			}
			else {
				worker->typed.workerData.targetConstruction = NO_TARGET_ID;
				game_unit_command_idle(worker);
			}
		}
		else {
			building_add_construction(context, workTarget);
		}
	} else {
		worker->typed.workerData.targetConstruction = NO_TARGET_ID;
		game_unit_command_idle(worker);
	}
	// TODO if we are harvesting
}

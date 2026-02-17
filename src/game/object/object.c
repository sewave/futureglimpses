#include "game/object/object.h"

#define FIRST_GENERATION 1
#define NO_FREE_INDEX -1
#define MOVE_PRECISION 16384
static unsigned short objectGenerations[MAX_GAME_UNITS];
static uint16_t nextFreeIndex;

typedef struct {
	ObjectTypeEnum type;
	uint8_t damageRadius;
	uint8_t minDamage;
	uint8_t maxDamage;
	uint16_t moveTime;
} ObjectData;

static ObjectData objectsData[OBJ_TYPE_NUMBER] = {
		{
				.type = OBJ_TYPE_ARROW,
				.damageRadius = 0,
				.minDamage = 6,
				.maxDamage = 10,
				.moveTime = SEC_TO_FRAMES(0.15),
		},
		{
				.type = OBJ_TYPE_FIREBALL,
				.damageRadius = 0,
				.minDamage = 0,
				.maxDamage = 0,
				.moveTime = SEC_TO_FRAMES(0.8),
		},
		{
				.type = OBJ_TYPE_EXPLOSION,
				.damageRadius = 1,
				.minDamage = 10,
				.maxDamage = 15,
				.moveTime = 0,
		},
		{
			.type = OBJ_TYPE_ARROW_DAMAGE,
			.damageRadius = 0,
			.minDamage = 6,
			.maxDamage = 10,
			.moveTime = 0,
		},
};

static int game_object_find_free_index(Object objects[]) {
	int startIndex = nextFreeIndex;
	int index = NO_FREE_INDEX;

	for (int i = 0; i < MAX_GAME_UNITS; i++) {
		int checkIndex = (startIndex + i) % MAX_GAME_UNITS;
		if (!objects[checkIndex].isActive) {
			index = checkIndex;
			nextFreeIndex = (index + 1) % MAX_GAME_UNITS;
			break;
		}
	}

	return index;
}

void game_objects_init(GameContext *context) {
	for (int i = 0; i < MAX_GAME_UNITS; i++) {
		context->objects[i].isActive = FALSE;
		context->objects[i].id = NULL_HANDLE;
		objectGenerations[i] = FIRST_GENERATION;
	}
	context->activeObjectsCount = 0;
	nextFreeIndex = 0;
}

Object *game_object_get_by_id(GameContext *context, ObjectId id) {
	int index = GET_INDEX(id);
	int gen = GET_GEN(id);

	if (index < 0 || index >= MAX_GAME_UNITS) return NULL;
	if (!context->objects[index].isActive) return NULL;
	if (objectGenerations[index] != gen) return NULL;

	return &context->objects[index];
}

void game_object_destroy(GameContext *context, ObjectId id) {
	Object *object = game_object_get_by_id(context, id);
	if (object && object->isActive) {
		object->isActive = FALSE;
		Object **activeList = context->activeObjects;
		for (int i = 0; i < context->activeObjectsCount; i++, activeList++) {
			if (*activeList == object) {
				*activeList = context->activeObjects[--context->activeObjectsCount];
				break;
			}
		}
	}
}

Object *game_object_spawn(GameContext *context, ObjectTypeEnum type, ControllerEnum controller, uint16_t sourceX, uint16_t sourceY, GameUnit* source, GameUnit *target, uint16_t targetX, uint16_t targetY) {
	int index = game_object_find_free_index(context->objects);
	if (index == NO_FREE_INDEX) return NULL;
	Object *object = &context->objects[index];
	if(source) {
		object->ownerId = source->id;
	} else {
		object->ownerId = NO_TARGET_ID;
	}
	objectGenerations[index]++;
	object->id = MAKE_ID(index, objectGenerations[index]);
	object->controller = controller;
	// Objects work in world coordinates
	object->x = sourceX;
	object->y = sourceY;
	object->currentX = object->x;
	object->currentY = object->y;
	object->moveTimeCounter = 0;
	object->isActive = TRUE;
	if (target) {
		object->targetId = target->id;
		object->targetX = target->x * TILE_SIZE + ((target->tileSize - 1) * TILE_SIZE) / 2;
		object->targetY = target->y * TILE_SIZE + ((target->tileSize - 1) * TILE_SIZE) / 2;
	} else {
		object->targetX = targetX * TILE_SIZE;
		object->targetY = targetY * TILE_SIZE;
	}
	// Direction towards target
	if (object->targetY < object->y) {
		if (object->targetX == object->x) {
			object->direction = OBJ_DIRECTION_NORTH;
		} else {
			if (object->targetX > object->x) {
				object->direction = OBJ_DIRECTION_NORTH_EAST;
			} else {
				object->direction = OBJ_DIRECTION_NORTH_WEST;
			}
		}
	} else {
		if (object->targetY > object->y) {
			if (object->targetX == object->x) {
				object->direction = OBJ_DIRECTION_SOUTH;
			} else {
				if (object->targetX > object->x) {
					object->direction = OBJ_DIRECTION_SOUTH_EAST;
				} else {
					object->direction = OBJ_DIRECTION_SOUTH_WEST;
				}
			}
		} else {
			if (object->targetX < object->x) {
				object->direction = OBJ_DIRECTION_WEST;
			} else {
				object->direction = OBJ_DIRECTION_EAST;
			}
		}
	}

	ObjectData *data = &objectsData[type];
	object->type = data->type;
	object->damageRadius = data->damageRadius;
	if(source) {
		uint16_t dx = object->targetX > object->x ? object->targetX / TILE_SIZE - object->x / TILE_SIZE : object->x / TILE_SIZE - object->targetX / TILE_SIZE; 
		uint16_t dy = object->targetY > object->y ? object->targetY / TILE_SIZE - object->y / TILE_SIZE : object->y / TILE_SIZE - object->targetY / TILE_SIZE;
		uint16_t maxDistance = source->maxAttackRange * source->maxAttackRange;
		object->moveTime = (data->moveTime * (dx * dx + dy * dy) / maxDistance);
		object->minDamage = source->minDamage;
		object->maxDamage = source->maxDamage;
	}
	else {
		object->moveTime = data->moveTime;
		object->minDamage = data->minDamage;
		object->maxDamage = data->maxDamage;
	}

	game_animation_object_set(object);
	game_gfx_set_object_sheet(object);
	// Add to active list
	context->activeObjects[context->activeObjectsCount++] = object;
	return object;
}

void game_objects_advance(GameContext *context) {
	int initialActiveObjects = context->activeObjectsCount;
	for (int i = 0; i < initialActiveObjects; i++) {
		Object *object = context->activeObjects[i];
		if (object->isActive) {
			game_animation_object_advance(context, object);
			if (object->moveTimeCounter < object->moveTime) {
				object->moveTimeCounter++;
				// Move towards target
				int t = (object->moveTimeCounter * MOVE_PRECISION) / object->moveTime;
				t = clamp(t, 0, MOVE_PRECISION);
				object->currentX = (object->x * (MOVE_PRECISION - t)) / MOVE_PRECISION + (object->targetX * t) / MOVE_PRECISION;
				object->currentY = (object->y * (MOVE_PRECISION - t)) / MOVE_PRECISION + (object->targetY * t) / MOVE_PRECISION;
			} else {
				if (game_animation_finished(&object->animationStatus)) {
					if (object->type == OBJ_TYPE_FIREBALL) {
						GameUnit* sourceUnit = game_unit_get_by_id(context, object->ownerId);
						game_object_spawn(context, OBJ_TYPE_EXPLOSION, object->controller, object->targetX, object->targetY,
										  sourceUnit, NULL, object->targetX / TILE_SIZE, object->targetY / TILE_SIZE);
					}
					else {
						if (object->type == OBJ_TYPE_ARROW) {
							GameUnit* targetUnit = game_unit_get_by_id(context, object->targetId);
							if(targetUnit) {
								GameUnit* sourceUnit = game_unit_get_by_id(context, object->ownerId);
								game_object_spawn(context, OBJ_TYPE_ARROW_DAMAGE, object->controller, targetUnit->x * TILE_SIZE + ((targetUnit->tileSize - 1) * TILE_SIZE) / 2, targetUnit->y * TILE_SIZE + ((targetUnit->tileSize - 1) * TILE_SIZE) / 2,
												  sourceUnit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
							}
						}
					}
					game_object_destroy(context, object->id);
				}
			}
		}
	}
}
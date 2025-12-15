#include "object.h"

#define FIRST_GENERATION 1
#define NO_FREE_INDEX -1
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
		.minDamage = 2,
		.maxDamage = 4,
		.moveTime = SEC_TO_FRAMES(0.5),
	},
	{
		.type = OBJ_TYPE_FIREBALL,
		.damageRadius = 1,
		.minDamage = 10,
		.maxDamage = 15,
		.moveTime = SEC_TO_FRAMES(1.2),
	}
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

Object *game_object_spawn(GameContext *context, UnitTypeEnum type, GameUnit* source, GameUnit* target, uint16_t targetX, uint16_t targetY) {
	int index = game_object_find_free_index(context->objects);
	if (index == NO_FREE_INDEX) return NULL;
	Object *object = &context->objects[index];
	objectGenerations[index]++;
	object->id = MAKE_ID(index, objectGenerations[index]);
	object->controller = source->controller;
	// Objects work in world coordinates
	object->x = source->x * TILE_SIZE;
	object->y = source->y * TILE_SIZE;
	object->moveTimeCounter = 0;
	object->isActive = TRUE;
	if(target) {
		object->targetId = target->id;
		object->targetX = target->x * TILE_SIZE;
		object->targetY = target->y * TILE_SIZE;
	} else {
		object->targetX = targetX * TILE_SIZE;
		object->targetY = targetY * TILE_SIZE;
	}
	// Direction towards target
	if(object->targetY < object->y) 
		object->direction = DIRECTION_NORTH;
	else {
		if(object->targetY > object->y) 
			object->direction = DIRECTION_SOUTH;
		else {
			if(object->targetX < object->x) 
				object->direction = DIRECTION_WEST;
			else {
				if(object->targetX > object->x) 
					object->direction = DIRECTION_EAST;
			}
		}
	}
	
	ObjectData* data = &objectsData[type];
	object->type = data->type;
	object->damageRadius = data->damageRadius;
	object->minDamage = data->minDamage;
	object->maxDamage = data->maxDamage;
	object->moveTime = data->moveTime;

	// TODO set animation and sheet
	/*game_animation_unit_set(object);
	game_gfx_set_sprite_sheet(object);*/
	// Add to active list
	context->activeObjects[context->activeObjectsCount++] = object;
	return object;
}

void game_objects_advance(GameContext *context) {
	for (int i = 0; i < context->activeObjectsCount; i++) {
		Object *object = context->activeObjects[i];
		if (object->isActive) {
			if (object->moveTimeCounter < object->moveTime) {
				object->moveTimeCounter++;
				// Move towards target
				int16_t dx = object->targetX - object->x;
				int16_t dy = object->targetY - object->y;
				if (dx != 0 || dy != 0) {
					float moveFraction = (float)object->moveTimeCounter / (float)object->moveTime;
					if (moveFraction > 1.0f) moveFraction = 1.0f;
					object->x = object->x + (int16_t)(dx * moveFraction);
					object->y = object->y + (int16_t)(dy * moveFraction);
				}
			} else {
				// Reached target
				// Process impact
				if (object->damageRadius > 0) {
					game_event_object_process(context, EVENT_TYPE_AREA_DAMAGE, object, 0);
				}
				else {
					game_event_object_process(context, EVENT_TYPE_DAMAGE, object, 0);
				}
				// Destroy object
				game_object_destroy(context, object->id);
			}
		}
	}
}
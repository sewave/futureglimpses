#ifndef OBJECT_H
#define OBJECT_H
#include "../game_lib.h"

Object *game_object_get_by_id(GameContext *context, ObjectId handle);
void game_object_destroy(GameContext *context, ObjectId handle);
void game_objects_init(GameContext *context);
Object *game_object_spawn(GameContext *context, ObjectTypeEnum type, ControllerEnum controller, uint16_t sourceX, uint16_t sourceY, GameUnit* target, uint16_t targetX, uint16_t targetY);
void game_objects_advance(GameContext *context);

#endif /* OBJECT_H */

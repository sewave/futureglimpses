#ifndef OBJECT_H
#define OBJECT_H
#include "../game_lib.h"

Object *game_object_get_by_id(GameContext *context, ObjectId handle);
void game_object_destroy(GameContext *context, ObjectId handle);
void game_objects_init(GameContext *context);
Object *game_object_spawn(GameContext *context, UnitTypeEnum type, GameUnit* source, GameUnit* target, uint16_t targetX, uint16_t targetY);

#endif /* OBJECT_H */

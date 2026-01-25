#ifndef UNIT_H
#define UNIT_H
#include "../game_lib.h"

#define UNIT_USED_RESOURCES 3
#define UNIT_CREATE_REDUCE_RESOURCES 2

typedef struct {
	uint16_t used[UNIT_USED_RESOURCES];
    uint8_t foodProvided;
	uint16_t time;
} UnitResourcesData;

typedef struct {
	UnitTypeEnum type;
	uint8_t isBuilding;
	uint8_t minAttackRange, maxAttackRange, sightRange;
	uint16_t health, maxHealth;
	uint8_t tileSize;
	uint8_t minDamage;
	uint8_t maxDamage;
	uint16_t reactionTime;
	uint16_t moveTime;
	UnitResourcesData resources;
} UnitData;

GameUnit *game_unit_get_by_id(GameContext *context, UnitId handle);
void game_unit_destroy(GameContext *context, UnitId handle);
void game_units_init(GameContext *context);
GameUnit *game_unit_spawn(GameContext *context, UnitTypeEnum type, ControllerEnum controller, uint16_t x, uint16_t y);
void game_unit_face_target(GameUnit* unit, GameUnit* target);
void game_unit_damage(GameContext *gameContext, uint8_t minDamage, uint8_t maxDamage, GameUnit* target);
void game_unit_area_damage(GameContext* context, Object* object);
void game_unit_process_all(GameContext *context);
UnitResourcesData* game_unit_get_resources(UnitTypeEnum type);
UnitData* game_unit_get_data(UnitTypeEnum type);
void game_unit_work(GameContext* context, GameUnit* worker);

#endif /* UNIT_H */

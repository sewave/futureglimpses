#ifndef MAP_H
#define MAP_H
#include "common/common.h"

#define CUSTOM_NAME_LENGTH 11
#define MAP_UPGRADEABLE_UNIT_TYPES 4
#define MAP_UNIT_CONTROLLERS_COUNT 2

typedef struct {
	uint8_t type;
	uint8_t controller;
	uint16_t x;
	uint16_t y;
	uint8_t isCustom;
	char name[CUSTOM_NAME_LENGTH];
	uint16_t maxHealth;
	uint8_t minDamage;
	uint8_t maxDamage;
	uint8_t mustSurvive;
	uint8_t armor;
	uint8_t padding[3];
} MapObject;

typedef struct {
	uint16_t numObjects;
	MapObject *objects;
} ObjectLayer;

typedef struct {
	uint16_t width;
	uint16_t height;
	uint16_t *tiles;
} TileLayer;

typedef struct {
	uint16_t numTileLayers;
	uint16_t numObjectLayers;
	uint32_t playerGold, playerWood;
	uint32_t computerGold, computerWood;

	char * title;
    char * description;
	char * win;
	char * lose;

	uint8_t enableBarracks;
	uint8_t enableBlacksmith;
	uint8_t enableCityHall;
	uint8_t enableFarm;
	uint8_t enableStables;
	uint8_t enableTower;
	uint8_t aiMode;
	uint16_t peaceTime;

	uint8_t upgradeableUnits[MAP_UPGRADEABLE_UNIT_TYPES];
	uint8_t upgradedUnits[MAP_UNIT_CONTROLLERS_COUNT][MAP_UPGRADEABLE_UNIT_TYPES];

	TileLayer *tileLayers;
	ObjectLayer *objectLayers;
} MapData;

void game_map_free_data(MapData *map);

MapData* game_map_load_data(const char *filename);

void game_map_load_campaign_metadata(const char *dirpath, char **title, char **description);

#endif /* MAP_H */

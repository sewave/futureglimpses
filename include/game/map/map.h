#ifndef MAP_H
#define MAP_H
#include "common/common.h"

#define CUSTOM_NAME_LENGTH 11

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

	TileLayer *tileLayers;
	ObjectLayer *objectLayers;
} MapData;

void game_map_free_data(MapData *map);

MapData* game_map_load_data(const char *filename);

void game_map_load_metadata(const char *filepath, char **title, char **description);

void game_map_load_campaign_metadata(const char *dirpath, char **title, char **description);

#endif /* MAP_H */

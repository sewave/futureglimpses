#ifndef SCENARIO_SELECT_MAP_H
#define SCENARIO_SELECT_MAP_H
#include "common/common.h"
#include "game/map/map.h"

#define MAPS_FOLDER "assets/map"

typedef enum {
	MAP_ENTRY_FILE,
	MAP_ENTRY_FOLDER,
	MAP_ENTRY_FOLDER_UP,
} MapEntryTypeEnum;

typedef struct {
	MapEntryTypeEnum type;
	char *path;
	char *title;
	char *description;
	uint8_t enableBarracks;
	uint8_t enableBlacksmith;
	uint8_t enableCityHall;
	uint8_t enableFarm;
	uint8_t enableStables;
	uint8_t enableTower;
	uint8_t enableTurret;
	uint8_t upgradeableUnits[MAP_UPGRADEABLE_UNIT_TYPES];
	uint8_t upgradedUnits[MAP_UPGRADEABLE_UNIT_TYPES];
} MapEntry;

typedef struct {
	MapEntry *entries;
	int count;
	int capacity;
} MapList;

int scenario_select_load_maps(const char *path, MapList **mapList);

void scenario_select_free_maps(MapList *mapList);

#endif /* SCENARIO_SELECT_MAP_H */

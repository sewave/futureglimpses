#ifndef SCENARIO_SELECT_MAP_H
#define SCENARIO_SELECT_MAP_H
#include "common/common.h"

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
} MapEntry;

typedef struct {
	MapEntry *entries;
	int count;
	int capacity;
} MapList;

int scenario_select_load_maps(const char *path, MapList **mapList);

void scenario_select_free_maps(MapList *mapList);

#endif /* SCENARIO_SELECT_MAP_H */

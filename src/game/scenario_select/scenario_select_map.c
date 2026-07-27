#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro.h>
#include "game/scenario/scenario_select_map.h"
#include "common/util.h"

#define MAX_MAPS 64
#define MAX_PATH_LENGTH 256
#define FGM_EXTENSION ".fgm"
#define DIR_FILTER "%s/*"
#define FGM_FILTER "%s/*.fgm"
#define PLAYER_CONTROLLER 0

typedef struct {
	MapList *mapList;
	const char *basePath;
} FileIteratorContext;

static int dir_iterator_callback(const char *filename, int attrib, void *param) {
	FileIteratorContext *ctx = (FileIteratorContext *) param;
	MapList *mapList = ctx->mapList;
	TRACE("Found file: %s (attrib=%d)\n", filename, attrib);
	if (mapList->count >= mapList->capacity) return 1;

	// Skip "." and ".." entries (check if path ends with these)
	size_t len = strlen(filename);
	if (len >= 2 && filename[len - 1] == '.' && filename[len - 2] != '.') return 0;
	uint8_t isUpFolder = FALSE;
	
	// ".." should be seen on upper folders
	if (len >= 2 && filename[len - 1] == '.' && filename[len - 2] == '.') {
		TRACE("Base path for ..: [%s], maps folder [%s]", ctx->basePath, MAPS_FOLDER);
		if(strcmp(ctx->basePath, MAPS_FOLDER) == 0) {
			return 0;
		} else {
			isUpFolder = TRUE;
		}
	}

	TRACE("Adding: %s (type=DIR)\n", filename);

	MapEntry *mapEntry = &mapList->entries[mapList->count];

	mapEntry->path = strdup(filename);
	if (!mapEntry->path) {
		TRACE("Error: Could not allocate memory for path.\n");
		exit(PROGRAM_ERROR);
	}
	if(isUpFolder) {
		get_parent_directory(ctx->basePath, mapEntry->path, strlen(mapEntry->path));
		mapEntry->type = MAP_ENTRY_FOLDER_UP;
		mapEntry->title = strdup("..");
		mapEntry->description = strdup("");
	}
	else {
		mapEntry->type = MAP_ENTRY_FOLDER;
		game_map_load_campaign_metadata(filename, &mapEntry->title, &mapEntry->description);
	}

	mapList->count++;
	return 0;
}

static int file_iterator_callback(const char *filename, int attrib, void *param) {
	FileIteratorContext *ctx = (FileIteratorContext *) param;
	MapList *mapList = ctx->mapList;
	TRACE("Found file: %s (attrib=%d)\n", filename, attrib);
	if (mapList->count >= mapList->capacity) return 1;

	const char *ext = strlwr(strrchr(filename, '.'));
	uint8_t isFgmFile = (ext != NULL && strcmp(ext, FGM_EXTENSION) == 0);

	if (!isFgmFile) return 0;

	TRACE("Adding: %s (type=FILE)\n", filename);

	MapEntry *mapEntry = &mapList->entries[mapList->count];

	mapEntry->path = strdup(filename);
	if (!mapEntry->path) {
		TRACE("Error: Could not allocate memory for path.\n");
		exit(PROGRAM_ERROR);
	}
	mapEntry->type = MAP_ENTRY_FILE;
	MapData* mapData = game_map_load_data(filename);
	if(!mapData) {
		TRACE("Error loading map data for %s\n", filename);
		exit(PROGRAM_ERROR);
	}
	mapEntry->title = strdup(mapData->title);
	mapEntry->description = strdup(mapData->description);

	mapEntry->enableBarracks = mapData->enableBarracks;
	mapEntry->enableBlacksmith = mapData->enableBlacksmith;
	mapEntry->enableCityHall = mapData->enableCityHall;
	mapEntry->enableFarm = mapData->enableFarm;
	mapEntry->enableStables = mapData->enableStables;
	mapEntry->enableTower = mapData->enableTower;
	mapEntry->enableTurret = mapData->enableTurret;
	for(int i = 0; i < MAP_UPGRADEABLE_UNIT_TYPES; i++) {
		mapEntry->upgradeableUnits[i] = mapData->upgradeableUnits[i];
		mapEntry->upgradedUnits[i] = mapData->upgradedUnits[PLAYER_CONTROLLER][i];
	}

	game_map_free_data(mapData);
	mapList->count++;
	return 0;
}

static int map_compare(const void *a, const void *b) {
	const MapEntry *entryA = (const MapEntry *) a;
	const MapEntry *entryB = (const MapEntry *) b;

	// Sort by type: FOLDER_UP < FOLDER < FILE
	if (entryA->type != entryB->type) {
		return (entryA->type == MAP_ENTRY_FOLDER_UP) ? -1 :
			   (entryB->type == MAP_ENTRY_FOLDER_UP) ? 1 :
			   (entryA->type == MAP_ENTRY_FOLDER) ? -1 : 1;
	}

	// If types are the same, sort by title
	return strcmp(entryA->title, entryB->title);
}

static void sort_map_entries(MapList* mapList) {
	if (!mapList || mapList->count <= 1) return;

	// Sort the entries by type first (folders up, then folders, then files), then by title string ascending
	qsort(mapList->entries, mapList->count, sizeof(MapEntry), map_compare);
}

int scenario_select_load_maps(const char *path, MapList **mapList) {
	if (!path || !mapList) {
		TRACE("Error: Invalid parameters to scenario_select_load_maps\n");
		exit(PROGRAM_ERROR);
	}

	scenario_select_free_maps(*mapList);
	*mapList = NULL;

	MapList *newMapList = (MapList *) malloc(sizeof(MapList));
	if (!newMapList) {
		TRACE("Error: Could not allocate memory for MapList.\n");
		exit(PROGRAM_ERROR);
	}

	newMapList->entries = (MapEntry *) malloc(sizeof(MapEntry) * MAX_MAPS);
	if (!newMapList->entries) {
		TRACE("Error: Could not allocate memory for map entries.\n");
		free(newMapList);
        newMapList = NULL;
		exit(PROGRAM_ERROR);
	}

	newMapList->count = 0;
	newMapList->capacity = MAX_MAPS;

	// Set up callback context
	FileIteratorContext ctx;
	ctx.mapList = newMapList;
	ctx.basePath = path;

	// Build search pattern for for_each_file_ex
	char pattern[MAX_PATH_LENGTH];
	snprintf(pattern, sizeof(pattern), DIR_FILTER, path);
	// Dirs and then files
	for_each_file_ex(pattern, FA_DIREC, FA_NONE, dir_iterator_callback, &ctx);
	snprintf(pattern, sizeof(pattern), FGM_FILTER, path);
	for_each_file_ex(pattern, FA_NONE, FA_DIREC | FA_HIDDEN, file_iterator_callback, &ctx);

	sort_map_entries(newMapList);

	*mapList = newMapList;

	return 0;
}

void scenario_select_free_maps(MapList *mapList) {
	if (!mapList) return;

	if (mapList->entries) {
		for (int i = 0; i < mapList->count; i++) {
			if (mapList->entries[i].path) {
				free(mapList->entries[i].path);
                mapList->entries[i].path = NULL;
			}
			if (mapList->entries[i].title) {
				free(mapList->entries[i].title);
                mapList->entries[i].title = NULL;
			}
			if (mapList->entries[i].description) {
				free(mapList->entries[i].description);
                mapList->entries[i].description = NULL;
			}
		}
		free(mapList->entries);
        mapList->entries = NULL;
	}

	free(mapList);
}

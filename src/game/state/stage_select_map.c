#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/state/stage_select_map.h"
#include "game/map/map.h"
#include <allegro.h>

typedef struct {
	MapList *mapList;
	const char *basePath;
} FileIteratorContext;

static int file_iterator_callback(const char *filename, int attrib, void *param) {
	FileIteratorContext *ctx = (FileIteratorContext *) param;
	MapList *mapList = ctx->mapList;

	if (mapList->count >= mapList->capacity) return 1;

	// Skip "." and ".." entries (check if path ends with these)
	size_t len = strlen(filename);
	if (len >= 1 && filename[len - 1] == '.') return 0;
	if (len >= 2 && filename[len - 1] == '.' && filename[len - 2] == '.') return 0;

	fprintf(stdout, "Found file: %s (attrib=%d)\n", filename, attrib);

	int isDirectory = (attrib & FA_DIREC) != 0;
	int isFgmFile = 0;

	if (!isDirectory) {
		const char *ext = strrchr(filename, '.');
		isFgmFile = (ext != NULL && strcmp(ext, ".fgm") == 0);
	}

	if (!isDirectory && !isFgmFile) return 0;

	fprintf(stdout, "Adding: %s (type=%s)\n", filename, isDirectory ? "DIR" : "FILE");

	mapList->entries[mapList->count].path = strdup(filename);
	if (!mapList->entries[mapList->count].path) {
		fprintf(stderr, "Error: Could not allocate memory for path\n");
		exit(PROGRAM_ERROR);
	}

	mapList->entries[mapList->count].type = isDirectory ? MAP_ENTRY_DIRECTORY : MAP_ENTRY_FILE;

	if (isDirectory) {
		game_map_load_campaign_metadata(filename,
										&mapList->entries[mapList->count].title,
										&mapList->entries[mapList->count].description);
	} else {
		game_map_load_metadata(filename,
							   &mapList->entries[mapList->count].title,
							   &mapList->entries[mapList->count].description);
	}

	mapList->count++;
	return 0;
}

int stage_select_load_maps(const char *path, MapList **mapList) {
	if (!path || !mapList) {
		fprintf(stderr, "Error: Invalid parameters to stage_select_load_maps\n");
		exit(PROGRAM_ERROR);
	}

	stage_select_free_maps(*mapList);
    *mapList = NULL;

    MapList *newMapList = (MapList *) malloc(sizeof(MapList));
	if (!newMapList) {
		fprintf(stderr, "Error: Could not allocate memory for MapList.\n");
		exit(PROGRAM_ERROR);
	}

	newMapList->entries = (MapEntry *) malloc(sizeof(MapEntry) * MAX_MAPS);
	if (!newMapList->entries) {
		fprintf(stderr, "Error: Could not allocate memory for map entries.\n");
		free(newMapList);
        newMapList = NULL;
		exit(PROGRAM_ERROR);
	}

	newMapList->count = 0;
	newMapList->capacity = MAX_MAPS;

	// Build search pattern for for_each_file_ex
	char pattern[256];
	snprintf(pattern, sizeof(pattern), "%s*", path);

	// Set up callback context
	FileIteratorContext ctx;
	ctx.mapList = newMapList;
	ctx.basePath = path;

    // Dirss and then files
	for_each_file_ex(pattern, FA_DIREC, 0, file_iterator_callback, &ctx);
	for_each_file_ex(pattern, FA_ARCH, 0, file_iterator_callback, &ctx);

	*mapList = newMapList;
	return 0;
}

void stage_select_free_maps(MapList *mapList) {
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

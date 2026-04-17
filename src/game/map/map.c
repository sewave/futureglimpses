#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/map/map.h"
#include "game/game_enums.h"

#define MAP_RESOURCES 4
#define MAP_RESOURCE_PLAYER_GOLD_INDEX 0
#define MAP_RESOURCE_PLAYER_WOOD_INDEX 1
#define MAP_RESOURCE_COMPUTER_GOLD_INDEX 2
#define MAP_RESOURCE_COMPUTER_WOOD_INDEX 3
#define MAP_OPTIONS 6
#define MAP_OPTION_ENABLE_BARRACKS_INDEX 0
#define MAP_OPTION_ENABLE_BLACKSMITH_INDEX 1
#define MAP_OPTION_ENABLE_FARM_INDEX 2
#define MAP_OPTION_ENABLE_STABLES_INDEX 3
#define MAP_OPTION_ENABLE_TOWER_INDEX 4
#define MAP_OPTION_AI_MODE_INDEX 5

#define MAP_UPGRADE_OPTIONS 12
#define MAP_UPGRADE_OPTIONS_ENABLED_INDEX 0
#define MAP_UPGRADE_OPTIONS_BLOCK 4

void game_map_free_data(MapData *map) {
	if (!map) return;

	free(map->title);
	map->title = NULL;
	free(map->description);
	map->description = NULL;

	free(map->win);
	map->win = NULL;

	free(map->lose);
	map->lose = NULL;


	if (map->tileLayers) {
		for (int i = 0; i < map->numTileLayers; i++) {
			if (map->tileLayers[i].tiles) {
				free(map->tileLayers[i].tiles);
				 map->tileLayers[i].tiles= NULL;
			}
		}
		free(map->tileLayers);
		map->tileLayers = NULL;
	}

	if (map->objectLayers) {
		for (int i = 0; i < map->numObjectLayers; i++) {
			if (map->objectLayers[i].objects) {
				free(map->objectLayers[i].objects);
				map->objectLayers[i].objects = NULL;
			}
		}
		free(map->objectLayers);
		map->objectLayers = NULL;
	}

	free(map);
}

/**
 * Helper function to read a string from the binary file. It first reads a uint16_t for the length,
 * then reads the characters and null-terminates the string.
 */
static char *read_string_from_file(FILE *file) {
	uint16_t length;
	if (fread(&length, sizeof(uint16_t), 1, file) != 1) {
		fprintf(stderr, "Error reading string length.\n");
		return NULL;
	}

	char *str = (char *) malloc(length + 1);
	if (!str) {
		fprintf(stderr, "Error allocating memory for string with length %d.\n", length);
		return NULL;
	}

	if (fread(str, sizeof(char), length, file) != length) {
		fprintf(stderr, "Error reading string data with length %d.\n", length);
		free(str);
		return NULL;
	}
	str[length] = '\0';
	return str;
}

static void *free_map_and_close_file(MapData *map, FILE *file) {
	game_map_free_data(map);
	fclose(file);
	return NULL;
}

static void cleanup_metadata_load_error(char **title, char **description, FILE *file) {
	*title = strdup("(error)");
	*description = strdup("(error)");
	if (file) fclose(file);
}

MapData *game_map_load_data(const char *filename) {
	FILE *filePtr = NULL;
	MapData *map = NULL;

	filePtr = fopen(filename, "rb");
	if (!filePtr) {
		fprintf(stderr, "Error opening the binary map file '%s'", filename);
		return NULL;
	}

	// 1. ALLOCATE AND READ THE HEADER (4 bytes)
	map = (MapData *) malloc(sizeof(MapData));
	if (!map) {
		fprintf(stderr, "Error: Could not allocate memory for MapData '%s'.\n", filename);
		fclose(filePtr);
		return NULL;
	}

	// Read the header directly into the structure
	if (fread(map, 2 * sizeof(uint16_t), 1, filePtr) != 1) {
		fprintf(stderr, "Error reading the map header.\n");
		game_map_free_data(map);
		fclose(filePtr);
		return NULL;
	}

	// If there are no layers, we stop here.
	if (map->numTileLayers == 0 && map->numObjectLayers == 0) {
		fclose(filePtr);
		return map;
	}

	// 2. LOAD TILE LAYERS
	if (map->numTileLayers > 0) {
		map->tileLayers = (TileLayer *) malloc(map->numTileLayers * sizeof(TileLayer));
		if (!map->tileLayers) {
			fprintf(stderr, "Error: Could not allocate memory for TileLayers array.\n");
			game_map_free_data(map);
			fclose(filePtr);
			return NULL;
		}

		for (int i = 0; i < map->numTileLayers; i++) {
			TileLayer *currentLayer = &map->tileLayers[i];

			// a. Read the layer header (width and height - 4 bytes)
			if (fread(currentLayer, 4, 1, filePtr) != 1) {
				fprintf(stderr, "Error reading TileLayer header %d.\n", i);
				game_map_free_data(map);
				fclose(filePtr);
				return NULL;
			}

			size_t tileCount = (size_t) currentLayer->width * currentLayer->height;
			size_t dataSize = tileCount * sizeof(uint16_t);

			// b. Allocate memory for the GIDs array
			currentLayer->tiles = (uint16_t *) malloc(dataSize);
			if (!currentLayer->tiles) {
				fprintf(stderr, "Error: Could not allocate memory for GIDs of layer %d.\n", i);
				game_map_free_data(map);
				fclose(filePtr);
				return NULL;
			}

			// c. Read all GIDs for the layer
			if (fread(currentLayer->tiles, sizeof(uint16_t), tileCount, filePtr) != tileCount) {
				fprintf(stderr, "Error reading GID data for layer %d. Expected %zu GIDs.\n", i, tileCount);
				game_map_free_data(map);
				fclose(filePtr);
				return NULL;
			}
		}
	}

	// 3. LOAD OBJECT LAYERS
	if (map->numObjectLayers > 0) {
		map->objectLayers = (ObjectLayer *) malloc(map->numObjectLayers * sizeof(ObjectLayer));
		if (!map->objectLayers) {
			fprintf(stderr, "Error: Could not allocate memory for ObjectLayers array.\n");
			game_map_free_data(map);
			fclose(filePtr);
			return NULL;
		}

		for (int i = 0; i < map->numObjectLayers; i++) {
			ObjectLayer *currentLayer = &map->objectLayers[i];

			// a. Read the number of objects (numObjects - 2 bytes)
			if (fread(&currentLayer->numObjects, sizeof(uint16_t), 1, filePtr) != 1) {
				fprintf(stderr, "Error reading object count in ObjectLayer %d.\n", i);
				game_map_free_data(map);
				fclose(filePtr);
				return NULL;
			}

			size_t object_count = currentLayer->numObjects;
			size_t dataSize = object_count * sizeof(MapObject);

			// b. Allocate memory for the objects array
			currentLayer->objects = (MapObject *) malloc(dataSize);
			if (!currentLayer->objects) {
				fprintf(stderr, "Error: Could not allocate memory for objects in layer %d.\n", i);
				game_map_free_data(map);
				fclose(filePtr);
				return NULL;
			}

			// c. Read all objects (6 bytes per object)
			if (fread(currentLayer->objects, sizeof(MapObject), object_count, filePtr) != object_count) {
				fprintf(stderr, "Error reading object data for layer %d. (Expected %zu)\n", i, object_count);
				game_map_free_data(map);
				fclose(filePtr);
				return NULL;
			}

			// Correct y position of object, 1 minus 1 to match tile indexing
			for (int j = 0; j < object_count; j++) {
				switch (currentLayer->objects[j].type) {
					case UNIT_TYPE_WORKER:
					case UNIT_TYPE_SOLDIER:
					case UNIT_TYPE_ARCHER:
					case UNIT_TYPE_KNIGHT:
					case UNIT_TYPE_MAGE:
						currentLayer->objects[j].y -= 1;
						break;
					case UNIT_TYPE_CITY_HALL:
					case UNIT_TYPE_BARRACKS:
					case UNIT_TYPE_STABLES:
						currentLayer->objects[j].y -= 3;
						break;
					case UNIT_TYPE_TOWER:
					case UNIT_TYPE_BLACKSMITH:
					case UNIT_TYPE_FARM:
						currentLayer->objects[j].y -= 2;
						break;
					default:
						// No adjustment needed
						break;
				}
			}
		}
	}

	// Load map strings
	map->title = read_string_from_file(filePtr);
	if (!map->title) return free_map_and_close_file(map, filePtr);
	map->description = read_string_from_file(filePtr);
	if (!map->description) return free_map_and_close_file(map, filePtr);
	map->win = read_string_from_file(filePtr);
	if (!map->win) return free_map_and_close_file(map, filePtr);
	map->lose = read_string_from_file(filePtr);
	if (!map->lose) return free_map_and_close_file(map, filePtr);

	// Load start resources, must be in sequence, gold wood gold wood
	uint32_t resources[MAP_RESOURCES];
	if (fread(resources, sizeof(uint32_t), MAP_RESOURCES, filePtr) != MAP_RESOURCES) {
		fprintf(stderr, "Error reading resources.\n");
		game_map_free_data(map);
		fclose(filePtr);
		return NULL;
	}
	map->playerGold = resources[MAP_RESOURCE_PLAYER_GOLD_INDEX];
	map->playerWood = resources[MAP_RESOURCE_PLAYER_WOOD_INDEX];
	map->computerGold = resources[MAP_RESOURCE_COMPUTER_GOLD_INDEX];
	map->computerWood = resources[MAP_RESOURCE_COMPUTER_WOOD_INDEX];

	// Read map options
	uint8_t mapOptions[MAP_OPTIONS];
	if (fread(mapOptions, sizeof(uint8_t), MAP_OPTIONS, filePtr) != MAP_OPTIONS) {
		fprintf(stderr, "Error reading map options.\n");
		game_map_free_data(map);
		fclose(filePtr);
		return NULL;
	}
	map->enableBarracks = mapOptions[MAP_OPTION_ENABLE_BARRACKS_INDEX];
	map->enableBlacksmith = mapOptions[MAP_OPTION_ENABLE_BLACKSMITH_INDEX];
	map->enableFarm = mapOptions[MAP_OPTION_ENABLE_FARM_INDEX];
	map->enableStables = mapOptions[MAP_OPTION_ENABLE_STABLES_INDEX];
	map->enableTower = mapOptions[MAP_OPTION_ENABLE_TOWER_INDEX];
	map->aiMode = mapOptions[MAP_OPTION_AI_MODE_INDEX];

	// Peace time
	if (fread(&map->peaceTime, sizeof(uint16_t), 1, filePtr) != 1) {
		fprintf(stderr, "Error reading map peace time.\n");
		game_map_free_data(map);
		fclose(filePtr);
		return NULL;
	}

	uint8_t upgradeMapOptions[MAP_UPGRADE_OPTIONS];
	if (fread(upgradeMapOptions, sizeof(uint8_t), MAP_UPGRADE_OPTIONS, filePtr) != MAP_UPGRADE_OPTIONS) {
		fprintf(stderr, "Error reading map upgrade options.\n");
		game_map_free_data(map);
		fclose(filePtr);
		return NULL;
	}

	for(int i = 0; i < MAP_UPGRADE_OPTIONS_BLOCK; i++) {
		map->upgradeableUnits[i] = upgradeMapOptions[MAP_UPGRADE_OPTIONS_ENABLED_INDEX + i];
	}

	for(int controller = 0; controller < UNIT_CONTROLLERS_COUNT; controller++) {
		for(int i = 0; i < MAP_UPGRADEABLE_UNIT_TYPES; i++) {
			map->upgradedUnits[controller][i] =
				upgradeMapOptions[MAP_UPGRADE_OPTIONS_BLOCK + controller * MAP_UPGRADEABLE_UNIT_TYPES + i];
		}
	}

	fclose(filePtr);
	return map;
}

void game_map_load_metadata(const char *filepath, char **title, char **description) {
	FILE *fp = fopen(filepath, "rb");
	if (!fp) {
		fprintf(stderr, "Warning: Could not open map file %s\n", filepath);
		cleanup_metadata_load_error(title, description, NULL);
		return;
	}

	// Read the header into a MapData structure
	MapData tempMap;
	if (fread(&tempMap, 2 * sizeof(uint16_t), 1, fp) != 1) {
		fprintf(stderr, "Warning: Could not read map header from %s\n", filepath);
		cleanup_metadata_load_error(title, description, fp);
		return;
	}

	uint16_t numTileLayers = tempMap.numTileLayers;
	uint16_t numObjectLayers = tempMap.numObjectLayers;

	// Skip tile layers
	for (int i = 0; i < numTileLayers; i++) {
		uint16_t layerDims[2];
		if (fread(layerDims, sizeof(uint16_t), 2, fp) != 2) {
			fprintf(stderr, "Warning: Could not read tile layer header\n");
			cleanup_metadata_load_error(title, description, fp);
			return;
		}
		uint16_t width = layerDims[0];
		uint16_t height = layerDims[1];
		size_t tileCount = width * height;
		// Skip all tile GIDs
		if (fseek(fp, tileCount * sizeof(uint16_t), SEEK_CUR) != 0) {
			fprintf(stderr, "Warning: Could not skip tile data\n");
			cleanup_metadata_load_error(title, description, fp);
			return;
		}
	}

	// Skip object layers
	for (int i = 0; i < numObjectLayers; i++) {
		uint16_t numObjects;
		if (fread(&numObjects, sizeof(uint16_t), 1, fp) != 1) {
			fprintf(stderr, "Warning: Could not read object count\n");
			cleanup_metadata_load_error(title, description, fp);
			return;
		}
		// Skip all objects
		if (fseek(fp, numObjects * sizeof(MapObject), SEEK_CUR) != 0) {
			fprintf(stderr, "Warning: Could not skip object data\n");
			cleanup_metadata_load_error(title, description, fp);
			return;
		}
	}

	// Now read title
	*title = read_string_from_file(fp);
	if (!title) {
		fprintf(stderr, "Warning: Could not read title from %s\n", filepath);
		cleanup_metadata_load_error(title, description, fp);
		return;
	}
	*description = read_string_from_file(fp);
	if (!description) {
		fprintf(stderr, "Warning: Could not read description from %s\n", filepath);
		cleanup_metadata_load_error(title, description, fp);
		return;
	}

	fclose(fp);
}

void game_map_load_campaign_metadata(const char *dirpath, char **title, char **description) {
	char filepath[512];
	snprintf(filepath, sizeof(filepath), "%s/campaign.txt", dirpath);

	FILE *fp = fopen(filepath, "r");
	if (!fp) {
		*title = strdup("");
		*description = strdup("");
		return;
	}

	char lineBuf[2048];
	if (fgets(lineBuf, sizeof(lineBuf), fp) != NULL) {
		size_t len = strlen(lineBuf);
		if (len > 0 && lineBuf[len - 1] == '\n') {
			lineBuf[len - 1] = '\0';
		}
		*title = strdup(lineBuf);
		if (!*title) {
			fprintf(stderr, "Warning: Could not allocate memory for title.\n");
			exit(PROGRAM_ERROR);
		}
	} else {
		*title = strdup("");
		*description = strdup("");
		fclose(fp);
		return;
	}

	if (fgets(lineBuf, sizeof(lineBuf), fp) != NULL) {
		size_t len = strlen(lineBuf);
		if (len > 0 && lineBuf[len - 1] == '\n') {
			lineBuf[len - 1] = '\0';
		}
		*description = strdup(lineBuf);
		if (!*description) {
			fprintf(stderr, "Warning: Could not allocate memory for description.\n");
			fclose(fp);
			exit(PROGRAM_ERROR);
		}
	} else {
		*description = strdup("");
	}

	fclose(fp);
}

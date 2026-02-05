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

void game_map_free_data(MapData *map) {
	if (!map) return;

	if (map->title) {
		free(map->title);
		map->title = NULL;
	}
	if (map->description) {
		free(map->description);
		map->description = NULL;
	}

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

MapData *game_map_load_data(const char *filename) {
	FILE *file_ptr = NULL;
	MapData *map = NULL;

	file_ptr = fopen(filename, "rb");
	if (!file_ptr) {
		fprintf(stderr, "Error opening the binary map file '%s'", filename);
		return NULL;
	}

	// 1. ALLOCATE AND READ THE HEADER (4 bytes)
	map = (MapData *) malloc(sizeof(MapData));
	if (!map) {
		fprintf(stderr, "Error: Could not allocate memory for MapData '%s'.\n", filename);
		fclose(file_ptr);
		return NULL;
	}

	// Read the header directly into the structure
	if (fread(map, 2 * sizeof(uint16_t), 1, file_ptr) != 1) {
		fprintf(stderr, "Error reading the map header.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}

	// If there are no layers, we stop here.
	if (map->numTileLayers == 0 && map->numObjectLayers == 0) {
		fclose(file_ptr);
		return map;
	}

	// 2. LOAD TILE LAYERS
	if (map->numTileLayers > 0) {
		map->tileLayers = (TileLayer *) malloc(map->numTileLayers * sizeof(TileLayer));
		if (!map->tileLayers) {
			fprintf(stderr, "Error: Could not allocate memory for TileLayers array.\n");
			game_map_free_data(map);
			fclose(file_ptr);
			return NULL;
		}

		for (int i = 0; i < map->numTileLayers; i++) {
			TileLayer *currentLayer = &map->tileLayers[i];

			// a. Read the layer header (width and height - 4 bytes)
			if (fread(currentLayer, 4, 1, file_ptr) != 1) {
				fprintf(stderr, "Error reading TileLayer header %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			size_t tileCount = (size_t) currentLayer->width * currentLayer->height;
			size_t dataSize = tileCount * sizeof(uint16_t);

			// b. Allocate memory for the GIDs array
			currentLayer->tiles = (uint16_t *) malloc(dataSize);
			if (!currentLayer->tiles) {
				fprintf(stderr, "Error: Could not allocate memory for GIDs of layer %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			// c. Read all GIDs for the layer
			if (fread(currentLayer->tiles, sizeof(uint16_t), tileCount, file_ptr) != tileCount) {
				fprintf(stderr, "Error reading GID data for layer %d. Expected %zu GIDs.\n", i, tileCount);
				game_map_free_data(map);
				fclose(file_ptr);
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
			fclose(file_ptr);
			return NULL;
		}

		for (int i = 0; i < map->numObjectLayers; i++) {
			ObjectLayer *currentLayer = &map->objectLayers[i];

			// a. Read the number of objects (numObjects - 2 bytes)
			if (fread(&currentLayer->numObjects, sizeof(uint16_t), 1, file_ptr) != 1) {
				fprintf(stderr, "Error reading object count in ObjectLayer %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			size_t object_count = currentLayer->numObjects;
			size_t dataSize = object_count * sizeof(MapObject);

			// b. Allocate memory for the objects array
			currentLayer->objects = (MapObject *) malloc(dataSize);
			if (!currentLayer->objects) {
				fprintf(stderr, "Error: Could not allocate memory for objects in layer %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			// c. Read all objects (6 bytes per object)
			if (fread(currentLayer->objects, sizeof(MapObject), object_count, file_ptr) != object_count) {
				fprintf(stderr, "Error reading object data for layer %d. (Expected %zu)\n", i, object_count);
				game_map_free_data(map);
				fclose(file_ptr);
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

	// 4. Load start resources, must be in sequence, gold wood gold wood
	uint32_t resources[MAP_RESOURCES];
	if (fread(resources, sizeof(uint32_t), MAP_RESOURCES, file_ptr) != MAP_RESOURCES) {
		fprintf(stderr, "Error reading resources.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}
	map->playerGold = resources[MAP_RESOURCE_PLAYER_GOLD_INDEX];
	map->playerWood = resources[MAP_RESOURCE_PLAYER_WOOD_INDEX];
	map->computerGold = resources[MAP_RESOURCE_COMPUTER_GOLD_INDEX];
	map->computerWood = resources[MAP_RESOURCE_COMPUTER_WOOD_INDEX];

	// 5. Load title and description strings as lines
	// Read title length
	uint16_t titleLength = 0;
	if (fread(&titleLength, sizeof(uint16_t), 1, file_ptr) != 1) {
		fprintf(stderr, "Error reading title length.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}
	// Allocate and read title string
	map->title = (char *) malloc(titleLength + 1);
	if (!map->title) {
		fprintf(stderr, "Error allocating memory for title string.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}
	if (fread(map->title, sizeof(char), titleLength, file_ptr) != titleLength) {
		fprintf(stderr, "Error reading title string.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}
	map->title[titleLength] = '\0';// Null-terminate the string

	// Read description length
	uint16_t descriptionLength = 0;
	if (fread(&descriptionLength, sizeof(uint16_t), 1, file_ptr) != 1) {
		fprintf(stderr, "Error reading description length.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}
	// Allocate and read description string
	map->description = (char *) malloc(descriptionLength + 1);
	if (!map->description) {
		fprintf(stderr, "Error allocating memory for description string.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}
	if (fread(map->description, sizeof(char), descriptionLength, file_ptr) != descriptionLength) {
		fprintf(stderr, "Error reading description string.\n");
		game_map_free_data(map);
		fclose(file_ptr);
		return NULL;
	}
	map->description[descriptionLength] = '\0';// Null-terminate the string

	fclose(file_ptr);
	return map;
}

void game_map_load_metadata(const char *filepath, char **title, char **description) {
	FILE *fp = fopen(filepath, "rb");
	if (!fp) {
		fprintf(stderr, "Warning: Could not open map file %s\n", filepath);
		*title = strdup("(error)");
		*description = strdup("");
		return;
	}

	// Read the header into a MapData structure
	MapData tempMap;
	if (fread(&tempMap, 2 * sizeof(uint16_t), 1, fp) != 1) {
		fprintf(stderr, "Warning: Could not read map header from %s\n", filepath);
		*title = strdup("(error)");
		*description = strdup("");
		fclose(fp);
		return;
	}

	uint16_t numTileLayers = tempMap.numTileLayers;
	uint16_t numObjectLayers = tempMap.numObjectLayers;

	// Skip tile layers
	for (int i = 0; i < numTileLayers; i++) {
		uint16_t layerDims[2];
		if (fread(layerDims, sizeof(uint16_t), 2, fp) != 2) {
			fprintf(stderr, "Warning: Could not read tile layer header\n");
			fclose(fp);
			*title = strdup("(error)");
			*description = strdup("");
			return;
		}
		uint16_t width = layerDims[0];
		uint16_t height = layerDims[1];
		size_t tileCount = width * height;
		// Skip all tile GIDs
		if (fseek(fp, tileCount * sizeof(uint16_t), SEEK_CUR) != 0) {
			fprintf(stderr, "Warning: Could not skip tile data\n");
			fclose(fp);
			*title = strdup("(error)");
			*description = strdup("");
			return;
		}
	}

	// Skip object layers
	for (int i = 0; i < numObjectLayers; i++) {
		uint16_t numObjects;
		if (fread(&numObjects, sizeof(uint16_t), 1, fp) != 1) {
			fprintf(stderr, "Warning: Could not read object count\n");
			fclose(fp);
			*title = strdup("(error)");
			*description = strdup("");
			return;
		}
		// Skip all objects
		if (fseek(fp, numObjects * sizeof(MapObject), SEEK_CUR) != 0) {
			fprintf(stderr, "Warning: Could not skip object data\n");
			fclose(fp);
			*title = strdup("(error)");
			*description = strdup("");
			return;
		}
	}

	// Skip resources (4 × 4 bytes = 16 bytes)
	if (fseek(fp, 4 * sizeof(uint32_t), SEEK_CUR) != 0) {
		fprintf(stderr, "Warning: Could not skip resources\n");
		fclose(fp);
		*title = strdup("(error)");
		*description = strdup("");
		return;
	}

	// Now read title
	uint16_t titleLength = 0;
	if (fread(&titleLength, sizeof(uint16_t), 1, fp) != 1) {
		fprintf(stderr, "Warning: Could not read title length from %s\n", filepath);
		*title = strdup("(no title)");
		*description = strdup("");
		fclose(fp);
		return;
	}

	char *titleBuf = (char *) malloc(titleLength + 1);
	if (!titleBuf) {
		fprintf(stderr, "Warning: Could not allocate memory for title\n");
		*title = strdup("(error)");
		*description = strdup("");
		fclose(fp);
		return;
	}

	if (fread(titleBuf, sizeof(char), titleLength, fp) != titleLength) {
		fprintf(stderr, "Warning: Could not read title data from %s\n", filepath);
		free(titleBuf);
		*title = strdup("(error)");
		*description = strdup("");
		fclose(fp);
		return;
	}
	titleBuf[titleLength] = '\0';
	*title = titleBuf;

	// Read description
	uint16_t descLength = 0;
	if (fread(&descLength, sizeof(uint16_t), 1, fp) != 1) {
		fprintf(stderr, "Warning: Could not read description length from %s\n", filepath);
		*description = strdup("");
		fclose(fp);
		return;
	}

	char *descBuf = (char *) malloc(descLength + 1);
	if (!descBuf) {
		fprintf(stderr, "Warning: Could not allocate memory for description\n");
		fclose(fp);
		exit(PROGRAM_ERROR);
	}

	if (fread(descBuf, sizeof(char), descLength, fp) != descLength) {
		fprintf(stderr, "Warning: Could not read description data from %s\n", filepath);
		free(descBuf);
		fclose(fp);
		exit(PROGRAM_ERROR);
	}
	descBuf[descLength] = '\0';
	*description = descBuf;

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

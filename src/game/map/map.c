#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "../game_enums.h"

void game_map_free_data(MapData *map) {
	if (!map) return;

	if (map->title) free(map->title);
	if (map->description) free(map->description);

	if (map->tileLayers) {
		for (int i = 0; i < map->numTileLayers; i++) {
			if (map->tileLayers[i].tiles) {
				free(map->tileLayers[i].tiles);
			}
		}
		free(map->tileLayers);
	}

	if (map->objectLayers) {
		for (int i = 0; i < map->numObjectLayers; i++) {
			if (map->objectLayers[i].objects) {
				free(map->objectLayers[i].objects);
			}
		}
		free(map->objectLayers);
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
	if (fread(map, 4, 1, file_ptr) != 1) {
		fprintf(stderr, "Error reading the header (4 bytes).\n");
		game_map_free_data(map);// Cleanup
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
			TileLayer *current_layer = &map->tileLayers[i];

			// a. Read the layer header (width and height - 4 bytes)
			if (fread(current_layer, 4, 1, file_ptr) != 1) {
				fprintf(stderr, "Error reading TileLayer header %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			size_t tile_count = (size_t) current_layer->width * current_layer->height;
			size_t data_size = tile_count * sizeof(unsigned short int);

			// b. Allocate memory for the GIDs array
			current_layer->tiles = (unsigned short int *) malloc(data_size);
			if (!current_layer->tiles) {
				fprintf(stderr, "Error: Could not allocate memory for GIDs of layer %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			// c. Read all GIDs for the layer
			if (fread(current_layer->tiles, sizeof(unsigned short int), tile_count, file_ptr) != tile_count) {
				fprintf(stderr, "Error reading GID data for layer %d. Expected %zu GIDs.\n", i, tile_count);
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
			ObjectLayer *current_layer = &map->objectLayers[i];

			// a. Read the number of objects (numObjects - 2 bytes)
			if (fread(&current_layer->numObjects, sizeof(unsigned short int), 1, file_ptr) != 1) {
				fprintf(stderr, "Error reading object count in ObjectLayer %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			size_t object_count = current_layer->numObjects;
			size_t data_size = object_count * sizeof(MapObject);

			// b. Allocate memory for the objects array
			current_layer->objects = (MapObject *) malloc(data_size);
			if (!current_layer->objects) {
				fprintf(stderr, "Error: Could not allocate memory for objects in layer %d.\n", i);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			// c. Read all objects (6 bytes per object)
			if (fread(current_layer->objects, sizeof(MapObject), object_count, file_ptr) != object_count) {
				fprintf(stderr, "Error reading object data for layer %d. (Expected %zu)\n", i, object_count);
				game_map_free_data(map);
				fclose(file_ptr);
				return NULL;
			}

			// Correct y position of object, 1 minus 1 to match tile indexing
			for (int j = 0; j < object_count; j++) {
				switch (current_layer->objects[j].type) {
					case UNIT_TYPE_WORKER:
					case UNIT_TYPE_SOLDIER:
					case UNIT_TYPE_ARCHER:
					case UNIT_TYPE_KNIGHT:
					case UNIT_TYPE_MAGE:
						current_layer->objects[j].y -= 1;
						break;
					case UNIT_TYPE_CITY_HALL:
					case UNIT_TYPE_BARRACKS:
					case UNIT_TYPE_STABLES:
						current_layer->objects[j].y -= 3;
						break;
					case UNIT_TYPE_TOWER:
					case UNIT_TYPE_BLACKSMITH:
					case UNIT_TYPE_FARM:
						current_layer->objects[j].y -= 2;
						break;
					default:
						// No adjustment needed
						break;
				}
			}
		}
	}

	// 4. Load title and description strings as lines
	// Read title length
	unsigned short int titleLength = 0;
	if (fread(&titleLength, sizeof(unsigned short int), 1, file_ptr) != 1) {
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
	unsigned short int descriptionLength = 0;
	if (fread(&descriptionLength, sizeof(unsigned short int), 1, file_ptr) != 1) {
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

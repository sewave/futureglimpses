#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

void game_map_free_data(MapData *map) {
    if (!map) return;

    if (map->tile_layers) {
        for (int i = 0; i < map->num_tile_layers; i++) {
            if (map->tile_layers[i].tiles) {
                free(map->tile_layers[i].tiles);
            }
        }
        free(map->tile_layers);
    }

    if (map->object_layers) {
        for (int i = 0; i < map->num_object_layers; i++) {
            if (map->object_layers[i].objects) {
                free(map->object_layers[i].objects);
            }
        }
        free(map->object_layers);
    }

    free(map);
}

MapData* game_map_load_data(const char *filename) {
    FILE *file_ptr = NULL;
    MapData *map = NULL;

    file_ptr = fopen(filename, "rb");
    if (!file_ptr) {
        fprintf(stderr, "Error opening the binary map file '%s'", filename);
        return NULL;
    }

    // 1. ALLOCATE AND READ THE HEADER (4 bytes)
    map = (MapData*)malloc(sizeof(MapData));
    if (!map) {
        fprintf(stderr, "Error: Could not allocate memory for MapData '%s'.\n", filename);
        fclose(file_ptr);
        return NULL;
    }
    
    // Read the header directly into the structure
    if (fread(map, 4, 1, file_ptr) != 1) { 
        fprintf(stderr, "Error reading the header (4 bytes).\n");
        game_map_free_data(map); // Cleanup
        fclose(file_ptr);
        return NULL;
    }
           
    // If there are no layers, we stop here.
    if (map->num_tile_layers == 0 && map->num_object_layers == 0) {
        fclose(file_ptr);
        return map;
    }

    // 2. LOAD TILE LAYERS
    if (map->num_tile_layers > 0) {
        map->tile_layers = (TileLayer*)malloc(map->num_tile_layers * sizeof(TileLayer));
        if (!map->tile_layers) {
            fprintf(stderr, "Error: Could not allocate memory for TileLayers array.\n");
            game_map_free_data(map);
            fclose(file_ptr);
            return NULL;
        }

        for (int i = 0; i < map->num_tile_layers; i++) {
            TileLayer *current_layer = &map->tile_layers[i];
            
            // a. Read the layer header (width and height - 4 bytes)
            if (fread(current_layer, 4, 1, file_ptr) != 1) {
                fprintf(stderr, "Error reading TileLayer header %d.\n", i);
                game_map_free_data(map);
                fclose(file_ptr);
                return NULL;
            }
            
            size_t tile_count = (size_t)current_layer->width * current_layer->height;
            size_t data_size = tile_count * sizeof(unsigned short int);
            
            // b. Allocate memory for the GIDs array
            current_layer->tiles = (unsigned short int*)malloc(data_size);
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
    if (map->num_object_layers > 0) {
        map->object_layers = (ObjectLayer*)malloc(map->num_object_layers * sizeof(ObjectLayer));
        if (!map->object_layers) {
            fprintf(stderr, "Error: Could not allocate memory for ObjectLayers array.\n");
            game_map_free_data(map);
            fclose(file_ptr);
            return NULL;
        }

        for (int i = 0; i < map->num_object_layers; i++) {
            ObjectLayer *current_layer = &map->object_layers[i];
            
            // a. Read the number of objects (num_objects - 2 bytes)
            if (fread(&current_layer->num_objects, sizeof(unsigned short int), 1, file_ptr) != 1) {
                fprintf(stderr, "Error reading object count in ObjectLayer %d.\n", i);
                game_map_free_data(map);
                fclose(file_ptr);
                return NULL;
            }
            
            size_t object_count = current_layer->num_objects;
            size_t data_size = object_count * sizeof(MapObject);

            // b. Allocate memory for the objects array
            current_layer->objects = (MapObject*)malloc(data_size);
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
        }
    }

    fclose(file_ptr);
    return map;
}

#ifndef MAP_H
#define MAP_H

typedef struct {
    unsigned char type;
    unsigned char controller;
    unsigned short int x;
    unsigned short int y;
} MapObject;

typedef struct {
    unsigned short int num_objects;
    MapObject *objects;
} ObjectLayer;

typedef struct {
    unsigned short int width;
    unsigned short int height;
    unsigned short int *tiles;
} TileLayer;

typedef struct {
    unsigned short int num_tile_layers;
    unsigned short int num_object_layers;
    
    TileLayer *tile_layers;
    ObjectLayer *object_layers;
} MapData;

/**
 * Frees all dynamically allocated memory for the map structure.
 * @param map Pointer to the MapData structure.
 */
void game_map_free_data(MapData *map);

/**
 * Loads the binary map data from a file into the MapData format in memory.
 *
 * @param filename The name of the binary map file.
 * @return A pointer to the loaded MapData structure, or NULL on error.
 */
MapData* game_map_load_data(const char *filename);

#endif /* MAP_H */

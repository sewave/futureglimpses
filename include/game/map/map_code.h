#ifndef MAP_CODE_H
#define MAP_CODE_H
#include <stdint.h>

#define MAP_CODE_LENGTH 8

typedef struct { char data[MAP_CODE_LENGTH + 1]; } MapCode; 
typedef struct {
	MapCode* codes;
	int codesQuantity;
} MapCodes;

MapCodes* map_code_read_all(const char* path);
uint8_t map_code_is_unlocked(MapCode* code, MapCodes* codes);
uint8_t map_code_is_master(MapCode* code);
MapCodes* map_code_add(MapCode* code, MapCodes* codes);
void map_code_merge_all(const char* path, MapCodes* codes);
void map_code_free(MapCodes* codes);

#endif // MAP_CODE_H

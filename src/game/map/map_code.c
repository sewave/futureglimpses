#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game/map/map_code.h"

#define MAP_CODE_FILE "unlocked.cod"
#define MASTER_CODE "        "

static char* map_code_get_full_path(const char* path) {
    char* fullPath = malloc(strlen(path) + strlen(MAP_CODE_FILE) + 2);
    sprintf(fullPath, "%s/%s", path, MAP_CODE_FILE);
    return fullPath;
}

MapCodes* map_code_read_all(const char* path) {
    // Read all map codes from path + map code file, each code is 8 bytes long and is a line in the file
    char* fullPath = map_code_get_full_path(path);
    FILE* file = fopen(fullPath, "r");
    if (file == NULL) {
        free(fullPath);
        // If the file does not exist, return master code as the only unlocked code
        MapCodes* codes = malloc(sizeof(MapCodes));
        codes->codesQuantity = 1;
        codes->codes = malloc(sizeof(MapCode));
        snprintf(codes->codes[0].data, MAP_CODE_LENGTH + 1, "%s", MASTER_CODE);
        return codes;
    }
    // Count the number of lines in the file
    int codesQuantity = 0;
    char line[MAP_CODE_LENGTH + 2]; // +2 for newline and null terminator
    while (fgets(line, sizeof(line), file) != NULL) {
        codesQuantity++;
    }
    // Allocate memory for the codes
    MapCodes* codes = malloc(sizeof(MapCodes));
    codes->codesQuantity = codesQuantity;
    codes->codes = malloc(sizeof(MapCode) * codesQuantity);
    // Read the codes from the file
    rewind(file);
    for (int i = 0; i < codesQuantity; i++) {
        fgets(line, sizeof(line), file);
        line[strcspn(line, "\n")] = '\0'; // Remove newline character
        strncpy(codes->codes[i].data, line, MAP_CODE_LENGTH);
        codes->codes[i].data[MAP_CODE_LENGTH] = '\0'; // Ensure null termination
    }
    fclose(file);
    free(fullPath);
    MapCode masterCode = {.data = MASTER_CODE};
    if(!map_code_is_unlocked(&masterCode, codes)) codes = map_code_add(&masterCode, codes);
    return codes;
}

void map_code_free(MapCodes* codes) {
    if (codes == NULL) {
        return;
    }
    if (codes->codes != NULL) {
        free(codes->codes);
    }
    free(codes);
}

uint8_t map_code_is_unlocked(MapCode* code, MapCodes* codes) {
    for (int i = 0; i < codes->codesQuantity; i++) {
        if (strncmp(code->data, codes->codes[i].data, MAP_CODE_LENGTH) == 0) {
            return -1;
        }
    }
    return 0;
}

uint8_t map_code_is_master(MapCode* code) {
    return strncmp(code->data, MASTER_CODE, MAP_CODE_LENGTH) == 0;
}

MapCodes* map_code_add(MapCode* code, MapCodes* codes) {
    // Check if the code is already in the list
    if (map_code_is_unlocked(code, codes)) {
        return codes;
    }
    // Allocate new memory for the codes
    MapCodes* newCodes = malloc(sizeof(MapCodes));
    newCodes->codesQuantity = codes->codesQuantity + 1;
    newCodes->codes = malloc(sizeof(MapCode) * newCodes->codesQuantity);
    // Copy old codes to new codes
    memcpy(newCodes->codes, codes->codes, sizeof(MapCode) * codes->codesQuantity);
    // Add the new code
    strncpy(newCodes->codes[newCodes->codesQuantity - 1].data, code->data, MAP_CODE_LENGTH);
    newCodes->codes[newCodes->codesQuantity - 1].data[MAP_CODE_LENGTH] = '\0'; // Ensure null termination
    // Free old codes
    map_code_free(codes);
    return newCodes;
}

void map_code_merge_all(const char* path, MapCodes* codes) {
    // Implementation for saving all map codes to a file
    char* fullPath = map_code_get_full_path(path);
    MapCodes* existingCodes = map_code_read_all(path);
    if (existingCodes != NULL) {
        // Open file for append and add only new codes
        FILE* file = fopen(fullPath, "a");
        if (file == NULL) {
            free(fullPath);
            map_code_free(existingCodes);
            return;
        }
        for (int i = 0; i < codes->codesQuantity; i++) {
            if (!map_code_is_unlocked(&codes->codes[i], existingCodes)) {
                fprintf(file, "%s\n", codes->codes[i].data);
            }
        }
        fclose(file);
    }
    else {
        FILE* file = fopen(fullPath, "w");
        if (file == NULL) {
            free(fullPath);
            return;
        }
        for (int i = 0; i < codes->codesQuantity; i++) {
            fprintf(file, "%s\n", codes->codes[i].data);
        }
        fclose(file);
        free(fullPath);
    }   
}

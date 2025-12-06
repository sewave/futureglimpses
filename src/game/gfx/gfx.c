#include "gfx.h"
#include <allegro/gfx.h>
#include <allegro/file.h>
#include <allegro/datafile.h>

static BITMAP* spriteSheetsBlue[UNIT_TYPE_NUMBER];
static BITMAP* spriteSheetsRed[UNIT_TYPE_NUMBER];

static const char* spriteSheetFilenamesBlue[UNIT_TYPE_NUMBER] = {
    "assets/gfx/unit/workerb.pcx",
    "assets/gfx/unit/swordfb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/knightb.pcx",
    "assets/gfx/unit/wizardb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
};

static const char* spriteSheetFilenamesRed[UNIT_TYPE_NUMBER] = {
    "assets/gfx/unit/workerr.pcx",
    "assets/gfx/unit/swordfr.pcx",
    "assets/gfx/unit/archerr.pcx",
    "assets/gfx/unit/knightr.pcx",
    "assets/gfx/unit/wizardr.pcx",
    "assets/gfx/unit/archerr.pcx",
    "assets/gfx/unit/archerr.pcx",
    "assets/gfx/unit/archerr.pcx",
    "assets/gfx/unit/archerr.pcx",
    "assets/gfx/unit/archerr.pcx",
    "assets/gfx/unit/archerr.pcx",
};

static uint8_t game_gfx_load_sprite_sheet(uchar index, BITMAP* spriteSheets[], const char * spriteSheetFilenames[]) {
    if (index >= UNIT_TYPE_NUMBER) return FALSE;
    if (spriteSheets[index] != NULL) destroy_bitmap(spriteSheets[index]);
    BITMAP* bitmap = load_bitmap(spriteSheetFilenames[index], NULL);
    if(!bitmap) return FALSE;
    spriteSheets[index] = bitmap;
    return TRUE;
}

InitializationStatusEnum game_gfx_load_sprite_sheets() {
    for(int i = 0; i < UNIT_TYPE_NUMBER; i++) {
        if(!game_gfx_load_sprite_sheet(i, spriteSheetsBlue, spriteSheetFilenamesBlue)) return INITIALIZATION_ERROR;
        if(!game_gfx_load_sprite_sheet(i, spriteSheetsRed, spriteSheetFilenamesRed)) return INITIALIZATION_ERROR;
    }
    return INITIALIZATION_OK;
}

void game_gfx_destroy_sprite_sheets() {
    for(int i = 0; i < UNIT_TYPE_NUMBER; i++) {
        if (spriteSheetsBlue[i] != NULL) destroy_bitmap(spriteSheetsBlue[i]);
        if (spriteSheetsRed[i] != NULL) destroy_bitmap(spriteSheetsRed[i]);
    }
}

void game_gfx_set_sprite_sheet(GameUnit* unit) {
    if(unit->controller == UNIT_CONTROLLER_PLAYER) {
        unit->animationStatus.sheet = spriteSheetsBlue[unit->type];
    } else {
        unit->animationStatus.sheet = spriteSheetsRed[unit->type];
    }
}

#include "gfx.h"
#include <allegro/gfx.h>
#include <allegro/file.h>
#include <allegro/datafile.h>

// TODO: blue/red
static BITMAP* spriteSheets[UNIT_TYPE_NUMBER];

static const char* spriteSheetFilenames[UNIT_TYPE_NUMBER] = {
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
    "assets/gfx/unit/archerb.pcx",
};

static void game_gfx_load_sprite_sheet(uchar index, BITMAP* bitmap) {
    if (index >= UNIT_TYPE_NUMBER) return;
    if (spriteSheets[index] != NULL) destroy_bitmap(spriteSheets[index]);
    spriteSheets[index] = bitmap;
}

InitializationStatusEnum game_gfx_load_sprite_sheets() {
    for(int i = 0; i < UNIT_TYPE_NUMBER; i++) {
        BITMAP* bitmap = load_bitmap(spriteSheetFilenames[i], NULL);
        if (bitmap == NULL) return INITIALIZATION_ERROR;
        game_gfx_load_sprite_sheet(i, bitmap);
    }
    return INITIALIZATION_OK;
}

void game_gfx_destroy_sprite_sheets() {
    for(int i = 0; i < UNIT_TYPE_NUMBER; i++) {
        if (spriteSheets[i] != NULL) destroy_bitmap(spriteSheets[i]);
    }
}

void game_gfx_set_sprite_sheet(GameUnit* unit) {
    unit->animationStatus.sheet = spriteSheets[unit->type];
}

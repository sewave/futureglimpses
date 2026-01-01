#include "gfx.h"
#include <allegro/gfx.h>
#include <allegro/file.h>
#include <allegro/datafile.h>

#define CMD_BAR_BUTTONS_FILE "assets/ui/cmdbtns.pcx"
#define FRAME_FILE "assets/ui/back.pcx"
#define TILESET_FILE "assets/gfx/tileset.pcx"
#define TILESET_COLORS_FILE "assets/gfx/tilesetm.pcx"
#define ICONS_FILE "assets/ui/icons.pcx"
#define OVERTILES_FILE "assets/ui/otiles.pcx"
#define ICON_WIDTH 8
#define ICON_HEIGHT 8

static BITMAP *spriteSheetsBlue[UNIT_TYPE_NUMBER];
static BITMAP *spriteSheetsRed[UNIT_TYPE_NUMBER];
static BITMAP *spriteSheetsObject[OBJ_TYPE_NUMBER];
static BITMAP *frame;
static BITMAP *cmdBarButtons;
static BITMAP *icons[GAME_ICON_COUNT];
static BITMAP *overtiles[GAME_OVERTILE_COUNT];
static BITMAP *tileSet;
static BITMAP *tileSetColors;

static const char *spriteSheetFilenamesBlue[UNIT_TYPE_NUMBER] = {
		"assets/gfx/unit/workerb.pcx",
		"assets/gfx/unit/swordfb.pcx",
		"assets/gfx/unit/archerb.pcx",
		"assets/gfx/unit/knightb.pcx",
		"assets/gfx/unit/wizardb.pcx",
		"assets/gfx/building/challb.pcx",
		"assets/gfx/building/farmb.pcx",
		"assets/gfx/building/barrackb.pcx",
		"assets/gfx/building/blasmib.pcx",
		"assets/gfx/building/stablesb.pcx",
		"assets/gfx/building/towerb.pcx",
};

static const char *spriteSheetFilenamesRed[UNIT_TYPE_NUMBER] = {
		"assets/gfx/unit/workerr.pcx",
		"assets/gfx/unit/swordfr.pcx",
		"assets/gfx/unit/archerr.pcx",
		"assets/gfx/unit/knightr.pcx",
		"assets/gfx/unit/wizardr.pcx",
		"assets/gfx/building/challr.pcx",
		"assets/gfx/building/farmr.pcx",
		"assets/gfx/building/barrackr.pcx",
		"assets/gfx/building/blasmir.pcx",
		"assets/gfx/building/stablesr.pcx",
		"assets/gfx/building/towerr.pcx",
};

static const char *objectSheetFilenames[OBJ_TYPE_NUMBER] = {
		"assets/gfx/object/arrow.pcx",
		"assets/gfx/object/fireball.pcx",
		"assets/gfx/object/explode.pcx",
		// TODO different sprite for arrow damage or null
		"assets/gfx/object/arrow.pcx",
};

static uint8_t game_gfx_load_sprite_sheet(uchar index, BITMAP *spriteSheets[], const char *spriteSheetFilenames[]) {
	if (index >= UNIT_TYPE_NUMBER) return FALSE;
	if (spriteSheets[index] != NULL) destroy_bitmap(spriteSheets[index]);
	BITMAP *bitmap = load_bitmap(spriteSheetFilenames[index], NULL);
	if (!bitmap) return FALSE;
	spriteSheets[index] = bitmap;
	return TRUE;
}

InitializationStatusEnum game_gfx_load_all() {
	printf("Loading units gfx [");
	for (int i = 0; i < UNIT_TYPE_NUMBER; i++) {
		if (!game_gfx_load_sprite_sheet(i, spriteSheetsBlue, spriteSheetFilenamesBlue)) return INITIALIZATION_ERROR;
		printInitStep();
		if (!game_gfx_load_sprite_sheet(i, spriteSheetsRed, spriteSheetFilenamesRed)) return INITIALIZATION_ERROR;
		printInitStep();
	}
	printOKSteps();

	printf("Loading objects gfx [");
	for (int i = 0; i < OBJ_TYPE_NUMBER; i++) {
		if (!game_gfx_load_sprite_sheet(i, spriteSheetsObject, objectSheetFilenames)) return INITIALIZATION_ERROR;
		printInitStep();
	}
	printOKSteps();

	printf("Loading ui gfx [");
	frame = load_bitmap(FRAME_FILE, NULL);
	if (!frame) return INITIALIZATION_ERROR;
	printInitStep();

	cmdBarButtons = load_bitmap(CMD_BAR_BUTTONS_FILE, NULL);
	if (!cmdBarButtons) return INITIALIZATION_ERROR;
	printInitStep();

	tileSet = load_bitmap(TILESET_FILE, NULL);
	if (!tileSet) return INITIALIZATION_ERROR;
	printInitStep();

	tileSetColors = load_bitmap(TILESET_COLORS_FILE, NULL);
	if (!tileSetColors) return INITIALIZATION_ERROR;
	printInitStep();

    BITMAP* allIcons = load_bitmap(ICONS_FILE, NULL);
    if (!allIcons) return INITIALIZATION_ERROR;
    for(int i = 0; i < GAME_ICON_COUNT; i++) {
        icons[i] = create_bitmap(ICON_WIDTH, ICON_HEIGHT);
        blit(allIcons, icons[i], i * ICON_WIDTH, 0, 0, 0, ICON_WIDTH, ICON_HEIGHT);
        printInitStep();
    }
    destroy_bitmap(allIcons);

    BITMAP* allOvertiles = load_bitmap(OVERTILES_FILE, NULL);
    if (!allOvertiles) return INITIALIZATION_ERROR;
    for(int i = 0; i < GAME_OVERTILE_COUNT; i++) {
        overtiles[i] = create_bitmap(TILE_SIZE, TILE_SIZE);
        blit(allOvertiles, overtiles[i], i * TILE_SIZE, 0, 0, 0, TILE_SIZE, TILE_SIZE);
        printInitStep();
    }
    destroy_bitmap(allOvertiles);

	printOKSteps();

	return INITIALIZATION_OK;
}

void game_gfx_destroy_all() {
	for (int i = 0; i < UNIT_TYPE_NUMBER; i++) {
		if (spriteSheetsBlue[i] != NULL) destroy_bitmap(spriteSheetsBlue[i]);
		if (spriteSheetsRed[i] != NULL) destroy_bitmap(spriteSheetsRed[i]);
	}
	for (int i = 0; i < OBJ_TYPE_NUMBER; i++) {
		if (spriteSheetsObject[i] != NULL) destroy_bitmap(spriteSheetsObject[i]);
	}
	if (frame) destroy_bitmap(frame);
	if (cmdBarButtons) destroy_bitmap(cmdBarButtons);
	if (tileSet) destroy_bitmap(tileSet);
	if (tileSetColors) destroy_bitmap(tileSetColors);
	for (int i = 0; i < GAME_ICON_COUNT; i++) {
		if (icons[i] != NULL) destroy_bitmap(icons[i]);
	}
	for (int i = 0; i < GAME_OVERTILE_COUNT; i++) {
		if (overtiles[i] != NULL) destroy_bitmap(overtiles[i]);
	}
}

void game_gfx_set_sprite_sheet(GameUnit *unit) {
	if (unit->controller == UNIT_CONTROLLER_PLAYER) {
		unit->animationStatus.sheet = spriteSheetsBlue[unit->type];
	} else {
		unit->animationStatus.sheet = spriteSheetsRed[unit->type];
	}
}

void game_gfx_set_object_sheet(Object *object) {
	object->animationStatus.sheet = spriteSheetsObject[object->type];
}

BITMAP *game_gfx_get_frame() {
	return frame;
}

BITMAP *game_gfx_get_icon(GameIconEnum gameIcon) {
	return icons[gameIcon];
}

BITMAP *game_gfx_get_cmd_bar_buttons() {
	return cmdBarButtons;
}

BITMAP *game_gfx_get_tileset() {
	return tileSet;
}

BITMAP *game_gfx_get_tileset_colors() {
	return tileSetColors;
}

BITMAP *game_gfx_get_unit_sheet(UnitTypeEnum type, ControllerEnum controller) {
	if (controller == UNIT_CONTROLLER_PLAYER) {
		return spriteSheetsBlue[type];
	} else {
		return spriteSheetsRed[type];
	}
}

BITMAP *game_gfx_get_overtile(GameOvertileEnum overtile) {
	return overtiles[overtile];
}

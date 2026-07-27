#include "game/video/gfx.h"
#include <allegro/gfx.h>
#include <allegro/file.h>
#include <allegro/datafile.h>
#include <stdio.h>
#include <common/console.h>

#define CMD_BAR_BUTTONS_FILE "assets/gfx/ui/cmdbtns.pcx"
#define FRAME_FILE "assets/gfx/ui/back/frame.pcx"
#define TILESET_FILE "assets/gfx/tileset.pcx"
#define TILESET_COLORS_FILE "assets/gfx/tilesetm.pcx"
#define ICONS_FILE "assets/gfx/ui/icons.pcx"
#define OVERTILES_FILE "assets/gfx/ui/otiles.pcx"
#define MENU_BACK_FILE "assets/gfx/ui/back/menuback.pcx"
#define UNIT_ICONS_FILE "assets/gfx/ui/unicons.pcx"
#define RESOURCE_HOVER_FILE "assets/gfx/ui/reshover.pcx"
#define ICON_WIDTH 8
#define ICON_HEIGHT 8

static SpriteSheet spriteSheetsBlue[UNIT_TYPE_NUMBER];
static SpriteSheet spriteSheetsRed[UNIT_TYPE_NUMBER];
static SpriteSheet spriteSheetsObject[OBJ_TYPE_NUMBER];
static BITMAP *frame;
static BITMAP *icons[GAME_ICON_COUNT];
static BITMAP *overtiles[GAME_OVERTILE_COUNT];
static RLE_SPRITE *unitIcons[GAME_UNIT_ICON_COUNT];
static RLE_SPRITE *cmdBarButtonIcons[CMD_BAR_BUTTON_ICON_COUNT];
static BITMAP *tileSet;
static BITMAP *tileSetColors;
static BITMAP *menuBack;
static BITMAP *resHover;

static const char *spriteSheetFilenamesBlue[UNIT_TYPE_NUMBER] = {
		[UNIT_TYPE_WORKER]		= "assets/gfx/unit/workerb.pcx",
		[UNIT_TYPE_SOLDIER]		= "assets/gfx/unit/swordfb.pcx",
		[UNIT_TYPE_ARCHER]		= "assets/gfx/unit/archerb.pcx",
		[UNIT_TYPE_KNIGHT]		= "assets/gfx/unit/knightb.pcx",
		[UNIT_TYPE_MAGE]		= "assets/gfx/unit/wizardb.pcx",
		[UNIT_TYPE_CITY_HALL]	= "assets/gfx/building/challb.pcx",
		[UNIT_TYPE_FARM]		= "assets/gfx/building/farmb.pcx",
		[UNIT_TYPE_BARRACKS]	= "assets/gfx/building/barrackb.pcx",
		[UNIT_TYPE_BLACKSMITH]	= "assets/gfx/building/blasmib.pcx",
		[UNIT_TYPE_STABLES]		= "assets/gfx/building/stablesb.pcx",
		[UNIT_TYPE_TOWER]		= "assets/gfx/building/towerb.pcx",
		[UNIT_TYPE_TURRET]		= "assets/gfx/building/turretb.pcx",
};

static const char *spriteSheetFilenamesRed[UNIT_TYPE_NUMBER] = {
		[UNIT_TYPE_WORKER]		= "assets/gfx/unit/workerr.pcx",
		[UNIT_TYPE_SOLDIER]		= "assets/gfx/unit/swordfr.pcx",
		[UNIT_TYPE_ARCHER]		= "assets/gfx/unit/archerr.pcx",
		[UNIT_TYPE_KNIGHT]		= "assets/gfx/unit/knightr.pcx",
		[UNIT_TYPE_MAGE]		= "assets/gfx/unit/wizardr.pcx",
		[UNIT_TYPE_CITY_HALL]	= "assets/gfx/building/challr.pcx",
		[UNIT_TYPE_FARM]		= "assets/gfx/building/farmr.pcx",
		[UNIT_TYPE_BARRACKS]	= "assets/gfx/building/barrackr.pcx",
		[UNIT_TYPE_BLACKSMITH]	= "assets/gfx/building/blasmir.pcx",
		[UNIT_TYPE_STABLES]		= "assets/gfx/building/stablesr.pcx",
		[UNIT_TYPE_TOWER]		= "assets/gfx/building/towerr.pcx",
		[UNIT_TYPE_TURRET]		= "assets/gfx/building/turretr.pcx",
};

static const char *objectSheetFilenames[OBJ_TYPE_NUMBER] = {
		[OBJ_TYPE_ARROW]		= "assets/gfx/object/arrow.pcx",
		[OBJ_TYPE_FIREBALL]		= "assets/gfx/object/fireball.pcx",
		[OBJ_TYPE_EXPLOSION]	= "assets/gfx/object/explode.pcx",
		[OBJ_TYPE_ARROW_DAMAGE]	= "assets/gfx/object/arrowhit.pcx",
		[OBJ_TYPE_FLAG]			= "assets/gfx/object/flag.pcx",
};

typedef struct {
	uint16_t width, height;
} SquareSize;

static const SquareSize unitSquares[UNIT_TYPE_NUMBER] = {
		// Mobile units
		[UNIT_TYPE_WORKER]		= {32, 32},
		[UNIT_TYPE_SOLDIER]		= {32, 32},
		[UNIT_TYPE_ARCHER]		= {32, 32},
		[UNIT_TYPE_KNIGHT]		= {32, 32},
		[UNIT_TYPE_MAGE]		= {32, 32},
		// Buildings
		[UNIT_TYPE_CITY_HALL]	= {48, 48},
		[UNIT_TYPE_FARM]		= {32, 32},
		[UNIT_TYPE_BARRACKS]	= {48, 48},
		[UNIT_TYPE_BLACKSMITH]	= {32, 32},
		[UNIT_TYPE_STABLES]		= {48, 48},
		[UNIT_TYPE_TOWER]		= {32, 32},
		[UNIT_TYPE_TURRET]		= {32, 32},
};

static const SquareSize objectSquares[OBJ_TYPE_NUMBER] = {
		[OBJ_TYPE_ARROW] 		= {16, 16},
		[OBJ_TYPE_FIREBALL] 	= {16, 16},
		[OBJ_TYPE_EXPLOSION]	= {48, 48},
		[OBJ_TYPE_ARROW_DAMAGE] = {16, 16},
		[OBJ_TYPE_FLAG]			= {16, 16},
};

static const SquareSize iconSquare = {8, 8};
static const SquareSize cmdButtonSquare = {32, 22};

static void game_gfx_destroy_sheet(SpriteSheet * spriteSheet) {
	if (spriteSheet->numFrames > 0 && spriteSheet->frames != NULL) {
		for(int i = 0; i < spriteSheet->numFrames; i++) destroy_rle_sprite(spriteSheet->frames[i]);
		free(spriteSheet->frames);
		spriteSheet->frames = NULL;
		spriteSheet->numFrames = 0;
	}
}

static RLE_SPRITE* game_gfx_get_rle_sprite_from_partial_bitmap(BITMAP *bitmap, SquareSize frameSize, int frameIndex) {
	int width = bitmap->w / frameSize.width;
	int x = (frameIndex % width) * frameSize.width;
	int y = (frameIndex / width) * frameSize.height;
	BITMAP* frameBitmap = create_bitmap(frameSize.width, frameSize.height);
	blit(bitmap, frameBitmap, x, y, 0, 0, frameSize.width, frameSize.height);
	RLE_SPRITE* sprite = get_rle_sprite(frameBitmap);
	destroy_bitmap(frameBitmap);
	return sprite;
}

static uint8_t game_gfx_load_sprite_sheet(SpriteSheet * spriteSheet, const char *spriteSheetFilename, SquareSize frameSize) {
	game_gfx_destroy_sheet(spriteSheet);
	BITMAP *framesBitmap = load_bitmap(spriteSheetFilename, NULL);
	if (!framesBitmap) return FALSE;
	int width = framesBitmap->w / frameSize.width;
	int height = framesBitmap->h / frameSize.height;
	spriteSheet->numFrames = width * height;
	spriteSheet->frames = (RLE_SPRITE **) calloc(spriteSheet->numFrames, sizeof(RLE_SPRITE*));

	for(int frame = 0; frame < spriteSheet->numFrames; frame++) {
		spriteSheet->frames[frame] = game_gfx_get_rle_sprite_from_partial_bitmap(framesBitmap, frameSize, frame);
	} 
	destroy_bitmap(framesBitmap);
	return TRUE;
}

InitializationStatusEnum game_gfx_load_all() {
	printf("Loading units gfx.................");
	ConsoleCoords cursorPos = console_get_cursor_position();
	common_print_load_step(cursorPos.x, cursorPos.y);
	for (int i = 0; i < UNIT_TYPE_NUMBER; i++) {
		if (!game_gfx_load_sprite_sheet(&spriteSheetsBlue[i], spriteSheetFilenamesBlue[i], unitSquares[i])) return INITIALIZATION_ERROR;
		common_print_load_step(cursorPos.x, cursorPos.y);
		if (!game_gfx_load_sprite_sheet(&spriteSheetsRed[i], spriteSheetFilenamesRed[i], unitSquares[i])) return INITIALIZATION_ERROR;
		common_print_load_step(cursorPos.x, cursorPos.y);
	}
	common_print_ok();

	printf("Loading unit icons................");
	cursorPos = console_get_cursor_position();
	common_print_load_step(cursorPos.x, cursorPos.y);
	BITMAP* unitIconsSheet = load_bitmap(UNIT_ICONS_FILE, NULL);
	if (!unitIconsSheet) return INITIALIZATION_ERROR;
	for (int i = 0; i < GAME_UNIT_ICON_COUNT; i++) {
		unitIcons[i] = game_gfx_get_rle_sprite_from_partial_bitmap(unitIconsSheet, iconSquare, i);
		common_print_load_step(cursorPos.x, cursorPos.y);
	}
	common_print_ok();
	destroy_bitmap(unitIconsSheet);

	printf("Loading objects gfx...............");
	cursorPos = console_get_cursor_position();
	common_print_load_step(cursorPos.x, cursorPos.y);
	for (int i = 0; i < OBJ_TYPE_NUMBER; i++) {
		if (!game_gfx_load_sprite_sheet(&spriteSheetsObject[i], objectSheetFilenames[i], objectSquares[i])) return INITIALIZATION_ERROR;
		common_print_load_step(cursorPos.x, cursorPos.y);
	}
	common_print_ok();

	printf("Loading command bar gfx...........");
	cursorPos = console_get_cursor_position();
	common_print_load_step(cursorPos.x, cursorPos.y);
	BITMAP *cmdBarButtons = load_bitmap(CMD_BAR_BUTTONS_FILE, NULL);
	if (!cmdBarButtons) return INITIALIZATION_ERROR;
	for (int i = 0; i < CMD_BAR_BUTTON_ICON_COUNT; i++) {
		cmdBarButtonIcons[i] = game_gfx_get_rle_sprite_from_partial_bitmap(cmdBarButtons, cmdButtonSquare, i);
		common_print_load_step(cursorPos.x, cursorPos.y);
	}
	common_print_ok();
	destroy_bitmap(cmdBarButtons);

	printf("Loading ui gfx....................");
	cursorPos = console_get_cursor_position();
	common_print_load_step(cursorPos.x, cursorPos.y);
	frame = load_bitmap(FRAME_FILE, NULL);
	if (!frame) return INITIALIZATION_ERROR;
	common_print_load_step(cursorPos.x, cursorPos.y);

	tileSet = load_bitmap(TILESET_FILE, NULL);
	if (!tileSet) return INITIALIZATION_ERROR;
	common_print_load_step(cursorPos.x, cursorPos.y);

	tileSetColors = load_bitmap(TILESET_COLORS_FILE, NULL);
	if (!tileSetColors) return INITIALIZATION_ERROR;
	common_print_load_step(cursorPos.x, cursorPos.y);

	BITMAP *allIcons = load_bitmap(ICONS_FILE, NULL);
	if (!allIcons) return INITIALIZATION_ERROR;
    for(int i = 0; i < GAME_ICON_COUNT; i++) {
        icons[i] = create_bitmap(ICON_WIDTH, ICON_HEIGHT);
        blit(allIcons, icons[i], i * ICON_WIDTH, 0, 0, 0, ICON_WIDTH, ICON_HEIGHT);
		common_print_load_step(cursorPos.x, cursorPos.y);
	}
	destroy_bitmap(allIcons);

    BITMAP* allOvertiles = load_bitmap(OVERTILES_FILE, NULL);
    if (!allOvertiles) return INITIALIZATION_ERROR;
    for(int i = 0; i < GAME_OVERTILE_COUNT; i++) {
        overtiles[i] = create_bitmap(TILE_SIZE, TILE_SIZE);
        blit(allOvertiles, overtiles[i], i * TILE_SIZE, 0, 0, 0, TILE_SIZE, TILE_SIZE);
		common_print_load_step(cursorPos.x, cursorPos.y);
	}
	destroy_bitmap(allOvertiles);

	menuBack = load_bitmap(MENU_BACK_FILE, NULL);
	if (!menuBack) return INITIALIZATION_ERROR;
	common_print_load_step(cursorPos.x, cursorPos.y);

	resHover = load_bitmap(RESOURCE_HOVER_FILE, NULL);
	if (!resHover) return INITIALIZATION_ERROR;
	common_print_ok();

	return INITIALIZATION_OK;
}

void game_gfx_destroy_all() {
	for (int i = 0; i < UNIT_TYPE_NUMBER; i++) {
		game_gfx_destroy_sheet(&spriteSheetsBlue[i]);
		game_gfx_destroy_sheet(&spriteSheetsRed[i]);
	}
	for (int i = 0; i < GAME_UNIT_ICON_COUNT; i++) {
		if (unitIcons[i] != NULL) {
			destroy_rle_sprite(unitIcons[i]);
			unitIcons[i] = NULL;
		}
	}
	for (int i = 0; i < OBJ_TYPE_NUMBER; i++) {
		game_gfx_destroy_sheet(&spriteSheetsObject[i]);
	}
	if (frame) destroy_bitmap(frame);
	if (tileSet) destroy_bitmap(tileSet);
	if (tileSetColors) destroy_bitmap(tileSetColors);
	for (int i = 0; i < GAME_ICON_COUNT; i++) {
		if (icons[i] != NULL) destroy_bitmap(icons[i]);
	}
	for (int i = 0; i < GAME_OVERTILE_COUNT; i++) {
		if (overtiles[i] != NULL) destroy_bitmap(overtiles[i]);
	}
	for (int i = 0; i < CMD_BAR_BUTTON_ICON_COUNT; i++) {
		if (cmdBarButtonIcons[i] != NULL) {
			destroy_rle_sprite(cmdBarButtonIcons[i]);
			cmdBarButtonIcons[i] = NULL;
		}
	}
}

void game_gfx_set_sprite_sheet(GameUnit *unit) {
	if (unit->controller == UNIT_CONTROLLER_PLAYER) {
		unit->animationStatus.sheet = &spriteSheetsBlue[unit->type];
	} else {
		unit->animationStatus.sheet = &spriteSheetsRed[unit->type];
	}
}

void game_gfx_set_object_sheet(Object *object) {
	object->animationStatus.sheet = &spriteSheetsObject[object->type];
}

RLE_SPRITE *game_gfx_get_object_sheet_frame(ObjectTypeEnum type, uint8_t frame) {
	return spriteSheetsObject[type].frames[frame];
}

BITMAP *game_gfx_get_frame() {
	return frame;
}

BITMAP *game_gfx_get_icon(GameIconEnum gameIcon) {
	return icons[gameIcon];
}

BITMAP *game_gfx_get_tileset() {
	return tileSet;
}

BITMAP *game_gfx_get_tileset_colors() {
	return tileSetColors;
}

SpriteSheet *game_gfx_get_unit_sheet(UnitTypeEnum type, ControllerEnum controller) {
	if (controller == UNIT_CONTROLLER_PLAYER) {
		return &spriteSheetsBlue[type];
	} else {
		return &spriteSheetsRed[type];
	}
}

BITMAP *game_gfx_get_overtile(GameOvertileEnum overtile) {
	return overtiles[overtile];
}

BITMAP *game_gfx_get_menu_back() {
	return menuBack;
}

RLE_SPRITE *game_gfx_get_unit_icon(GameUnitIconEnum unitIcon) {
	return unitIcons[unitIcon];
}

BITMAP *game_gfx_get_resource_hover() {
	return resHover;
}

RLE_SPRITE *game_gfx_get_cmd_bar_button_icon(CommandBarButtonIconEnum btnType) {
	return cmdBarButtonIcons[btnType];
}

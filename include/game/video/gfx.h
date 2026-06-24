#ifndef GAME_GFX_H
#define GAME_GFX_H
#include "game/game.h"

typedef enum {
	GAME_ICON_GOLD,
	GAME_ICON_WOOD,
	GAME_ICON_FOOD,
	GAME_ICON_CHECK_OFF,
	GAME_ICON_CHECK_ON,
	GAME_ICON_OPTION_OFF,
	GAME_ICON_OPTION_ON,
	GAME_ICON_BAR_LEFT_OFF,
	GAME_ICON_BAR_LEFT_ON,
	GAME_ICON_BAR_RIGHT_OFF,
	GAME_ICON_BAR_RIGHT_ON,
	GAME_ICON_BAR,
	GAME_ICON_BAR_UP_OFF,
	GAME_ICON_BAR_UP_ON,
	GAME_ICON_BAR_DOWN_OFF,
	GAME_ICON_BAR_DOWN_ON,
	GAME_ICON_BAR_VERTICAL,
	GAME_ICON_FOLDER,
	GAME_ICON_MAP,
	GAME_ICON_FOLDER_UP,
	GAME_ICON_COUNT,
} GameIconEnum;

typedef enum {
	GAME_OVERTILE_OK,
	GAME_OVERTILE_KO,
	GAME_OVERTILE_COUNT,
} GameOvertileEnum;

typedef enum {
	GAME_UNIT_ICON_GOLD,
	GAME_UNIT_ICON_WOOD,
	GAME_UNIT_ICON_CUSTOM,
	GAME_UNIT_ICON_FUTURE,
	GAME_UNIT_ICON_ATTACK,
	GAME_UNIT_ICON_ARMOR,
	GAME_UNIT_ICON_RANGE,
	GAME_UNIT_ICON_QUEUE,
	GAME_UNIT_ICON_COUNT,
} GameUnitIconEnum;

InitializationStatusEnum game_gfx_load_all();
void game_gfx_destroy_all();
void game_gfx_set_sprite_sheet(GameUnit *unit);
void game_gfx_set_object_sheet(Object *object);
BITMAP *game_gfx_get_frame();
BITMAP *game_gfx_get_icon(GameIconEnum gameIcon);
RLE_SPRITE *game_gfx_get_unit_icon(GameUnitIconEnum unitIcon);
BITMAP *game_gfx_get_tileset();
BITMAP *game_gfx_get_tileset_colors();
SpriteSheet *game_gfx_get_unit_sheet(UnitTypeEnum type, ControllerEnum controller);
BITMAP *game_gfx_get_overtile(GameOvertileEnum overtile);
BITMAP *game_gfx_get_menu_back();
BITMAP *game_gfx_get_resource_hover();
RLE_SPRITE *game_gfx_get_cmd_bar_button_icon(CommandBarButtonIconEnum btnType);
RLE_SPRITE *game_gfx_get_object_sheet_frame(ObjectTypeEnum type, uint8_t frame);

#endif /* GAME_GFX_H */

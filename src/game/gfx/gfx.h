#ifndef GAME_GFX_H
#define GAME_GFX_H
#include "../game_lib.h"

typedef enum {
	GAME_ICON_GOLD,
	GAME_ICON_WOOD,
	GAME_ICON_FOOD,
	GAME_ICON_COUNT,
} GameIconEnum;

typedef enum {
	GAME_OVERTILE_OK,
	GAME_OVERTILE_KO,
	GAME_OVERTILE_COUNT,
} GameOvertileEnum;

InitializationStatusEnum game_gfx_load_all();
void game_gfx_destroy_all();
void game_gfx_set_sprite_sheet(GameUnit *unit);
void game_gfx_set_object_sheet(Object *object);
BITMAP *game_gfx_get_frame();
BITMAP *game_gfx_get_icon(GameIconEnum gameIcon);
BITMAP *game_gfx_get_cmd_bar_buttons();
BITMAP *game_gfx_get_tileset();
BITMAP *game_gfx_get_tileset_colors();
BITMAP *game_gfx_get_unit_sheet(UnitTypeEnum type, ControllerEnum controller);
BITMAP *game_gfx_get_overtile(GameOvertileEnum overtile);

#endif /* GAME_GFX_H */

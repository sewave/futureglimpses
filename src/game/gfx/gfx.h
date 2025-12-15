#ifndef GAME_GFX_H
#define GAME_GFX_H
#include "../game_lib.h"

InitializationStatusEnum game_gfx_load_sprite_sheets();
void game_gfx_destroy_sprite_sheets();
void game_gfx_set_sprite_sheet(GameUnit* unit);
void game_gfx_set_object_sheet(Object* object);

#endif

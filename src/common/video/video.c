#include <allegro/gfx.h>
#include "../common.h"

int video_init_system(int width, int height, int depth) {
	set_color_depth(depth);
#ifdef DOS
	if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, width, height, 0, 0) != ALLEGRO_INIT_OK) {
#endif
		if (set_gfx_mode(GFX_SAFE, width, height, 0, 0) != ALLEGRO_INIT_OK) {
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			return INITIALIZATION_ERROR;
		}
#ifdef DOS
	}
#endif
	return INITIALIZATION_OK;
}
#include <allegro/gfx.h>
#include <allegro/palette.h>
#include "game_video.h"

#define FIRST_COLORS 32

static RGB firstColors[FIRST_COLORS] = {
    // TRANSPARENT
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // WHITE
    {.r = 255, .g = 255, .b = 255, .filler = 0},
    // BLACK
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // GREEN
    {.r = 0, .g = 255, .b = 0, .filler = 0},
    // YELLOW
    {.r = 255, .g = 255, .b = 0, .filler = 0},
    // RED
    {.r = 255, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // UNUSED
    {.r = 0, .g = 0, .b = 0, .filler = 0},
};

// TODO last colors

InitializationStatusEnum game_video_load_universal_pal() {
	PALETTE universalPal;
	if(video_load_raw_palette("assets/pal/game.pal", universalPal) != PROGRAM_OK) {
		return PROGRAM_ERROR;
	}
	// Load fixed palette indexes for ui
    for(int i = 0; i < FIRST_COLORS; i++) {
        universalPal[i] = firstColors[i];
    }

	set_palette(universalPal);
	
	return PROGRAM_OK;
}
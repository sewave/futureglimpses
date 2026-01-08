#include <allegro/palette.h>
#include <allegro/color.h>
#include "game_video.h"

#define FIRST_COLORS 32
#define LAST_COLORS 8
#define TOTAL_COLORS 256
#define LAST_COLORS_INDEX TOTAL_COLORS - LAST_COLORS

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
    // GRAY
    {.r = 153, .g = 153, .b = 153, .filler = 0},
    // DARK_GREEN
    {.r = 0, .g = 153, .b = 0, .filler = 0},
    // DARK_GRAY
    {.r =76, .g = 76, .b = 76, .filler = 0},
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

// Last colors
static RGB lastColors[LAST_COLORS] = {
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

InitializationStatusEnum game_video_load_universal_pal() {
	PALETTE universalPal;
	if(video_load_raw_palette("assets/pal/game.pal", universalPal) != PROGRAM_OK) return PROGRAM_ERROR;
	// Load fixed palette indexes for ui
    for(int i = 0; i < FIRST_COLORS; i++) universalPal[i] = firstColors[i];
    for(int i = LAST_COLORS_INDEX; i < LAST_COLORS; i++) universalPal[i + LAST_COLORS_INDEX] = lastColors[i];

	set_palette(universalPal);
	
	return PROGRAM_OK;
}
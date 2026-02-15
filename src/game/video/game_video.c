#include <allegro/palette.h>
#include <allegro/color.h>
#include "game/video/game_video.h"

#define FIRST_COLORS 32
#define LAST_COLORS 8
#define TOTAL_COLORS 256
#define LAST_COLORS_INDEX TOTAL_COLORS - LAST_COLORS

static RGB firstColors[FIRST_COLORS] = {
    // TRANSPARENT
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // WHITE
    {.r = 63, .g = 63, .b = 63, .filler = 0},
    // BLACK
    {.r = 0, .g = 0, .b = 0, .filler = 0},
    // GREEN
    {.r = 0, .g = 63, .b = 0, .filler = 0},
    // YELLOW
    {.r = 63, .g = 63, .b = 0, .filler = 0},
    // RED
    {.r = 63, .g = 0, .b = 0, .filler = 0},
    // GRAY
    {.r = 38, .g = 38, .b = 38, .filler = 0},
    // DARK_GREEN
    {.r = 0, .g = 38, .b = 0, .filler = 0},
    // DARK_GRAY
    {.r = 19, .g = 19, .b = 19, .filler = 0},
    // TURQUOISE
    {.r = 20, .g = 40, .b = 38, .filler = 0},
    // DARK_TURQUOISE
    {.r = 0, .g = 16, .b = 16, .filler = 0},
    // CMD_BUTTON
    {.r = 0, .g = 26, .b = 38, .filler = 0},
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

InitializationStatusEnum game_video_load_universal_pal(PALETTE palette) {
	if(video_load_raw_palette("assets/pal/game.pal", palette) != PROGRAM_OK) return PROGRAM_ERROR;
	// Load fixed palette indexes for ui
    for(int i = 0; i < FIRST_COLORS; i++) palette[i] = firstColors[i];
    for(int i = LAST_COLORS_INDEX; i < LAST_COLORS; i++) palette[i + LAST_COLORS_INDEX] = lastColors[i];

	return PROGRAM_OK;
}

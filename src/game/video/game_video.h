#ifndef GAME_VIDEO_H
#define GAME_VIDEO_H
#include "../game_lib.h"

#define GAME_INTERNAL_WIDTH 320
#define GAME_INTERNAL_HEIGHT 200
#ifdef DOS
#define GAME_EXTERNAL_WIDTH GAME_INTERNAL_WIDTH
#define GAME_EXTERNAL_HEIGHT GAME_INTERNAL_HEIGHT
#else
#define GAME_EXTERNAL_WIDTH GAME_INTERNAL_WIDTH * 3
#define GAME_EXTERNAL_HEIGHT GAME_INTERNAL_HEIGHT * 3
#endif

#define GAME_COLOR_DEPTH 8

#define BACKGROUND_Z_ORDER 0
#define SPRITES_Z_ORDER 1000
#define UI_Z_ORDER 2000
#define MOUSE_Z_ORDER 3000

#define UNIVERSAL_PAL_FIRST_INDEX 32
#define UNIVERSAL_PAL_LAST_INDEX 247

#ifdef DOS
    #define PAL_COLOR_TRANS 0
    #define PAL_COLOR_WHITE 1
    #define PAL_COLOR_BLACK 2
    #define PAL_COLOR_GREEN 3
    #define PAL_COLOR_YELLOW 4
    #define PAL_COLOR_RED 5
#else
    #define PAL_COLOR_TRANS makecol(0, 0, 0)
    #define PAL_COLOR_WHITE makecol(255, 255, 255)
    #define PAL_COLOR_BLACK makecol(0, 0, 0)
    #define PAL_COLOR_GREEN makecol(0, 255, 0)
    #define PAL_COLOR_YELLOW makecol(255, 255, 0)
    #define PAL_COLOR_RED makecol(255, 0, 0)
#endif

InitializationStatusEnum game_video_load_universal_pal();

#endif /* GAME_VIDEO_H */

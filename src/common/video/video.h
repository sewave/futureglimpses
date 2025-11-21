#ifndef VIDEO_H
#define VIDEO_H
#include <allegro/palette.h>
#include <stdio.h>
#include "../common.h"

typedef struct RGB_FILE_ENTRY {
    unsigned char r, g, b;
} RGB_FILE_ENTRY;

#define PALETTE_ENTRIES 256

int video_init_system(int width, int height, int depth);
int video_load_raw_palette(const char *filename, PALETTE p);
#endif /* VIDEO_H */

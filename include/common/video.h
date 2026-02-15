#ifndef VIDEO_H
#define VIDEO_H
#include <allegro/palette.h>
#include "common/common.h"

typedef struct RGB_FILE_ENTRY {
    unsigned char r, g, b;
} RGB_FILE_ENTRY;

#define TRANSPARENT_INDEX -1
#define TRANSPARENT_COLOR 0
#define PALETTE_ENTRIES 256

InitializationStatusEnum video_init_system(int width, int height, int depth);
InitializationStatusEnum video_load_raw_palette(const char *filename, PALETTE p);
void video_fade_reset();
void video_fade_in_init(uint8_t speed, PALETTE palette);
void video_fade_out_init(uint8_t speed);
void video_fade_handle();

#endif /* VIDEO_H */

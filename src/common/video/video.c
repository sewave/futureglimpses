#include <allegro/gfx.h>
#include <allegro/color.h>
#include "common/video.h"
#include <stdio.h>

typedef enum {
	FADE_NONE,
	FADE_IN,
	FADE_OUT
} FadeEnum;

static RGB *fadePalette;
static FadeEnum fadeType;
static uint8_t fadeSpeed;
static uint8_t skipFadeIn;

static int video_get_color_distance(RGB *source, RGB *target) {
	return abs(source->r - target->r) + abs(source->g - target->g) + abs(source->b - target->b);
}

InitializationStatusEnum video_init_system(int width, int height, int depth) {
	printf("Initializing video...");
	set_color_depth(depth);
#ifdef DOS
	if (set_gfx_mode(GFX_AUTODETECT, width, height, 0, 0) != ALLEGRO_INIT_OK) {
#endif
		if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, width, height, 0, 0) != ALLEGRO_INIT_OK) {
			set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
			return INITIALIZATION_ERROR;
		}
#ifdef DOS
	}
#endif
	return INITIALIZATION_OK;
}

InitializationStatusEnum video_load_raw_palette(const char *filename, PALETTE p) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error: Could not open palette file '%s'\n", filename);
        return FUNCTION_ERROR;
    }
    
    RGB_FILE_ENTRY fileEntry;
    RGB *currentRgb = p; 
    for (int i = 0; i < PALETTE_ENTRIES; ++i, ++currentRgb) {
        if (fread(&fileEntry, sizeof(RGB_FILE_ENTRY), 1, fp) != 1) {
            printf("Error reading color entry %d from file '%s'.\n", i, filename);
            fclose(fp);
            return FUNCTION_ERROR;
        }
        currentRgb->r = fileEntry.r;
        currentRgb->g = fileEntry.g;
        currentRgb->b = fileEntry.b;
        currentRgb->filler = 0; 
    }
    fclose(fp);
    return FUNCTION_OK;
}

void video_fade_reset() {
    fadeType = FADE_NONE;
    fadePalette = NULL;
    fadeSpeed = 64;
	skipFadeIn = FALSE;
}

void video_fade_handle() {
	if (fadeType == FADE_NONE) return;
	vsync();
	if (fadeType == FADE_IN && fadePalette) {
		fade_in(fadePalette, fadeSpeed);
	} else {
		if (fadeType == FADE_OUT) fade_out(fadeSpeed);
	}
	fadeType = FADE_NONE;
}

void video_fade_in_init(uint8_t speed, PALETTE palette) {
	if(skipFadeIn) {
		skipFadeIn = FALSE;
		return;
	}
	fadeType = FADE_IN;
	fadeSpeed = speed;
	fadePalette = palette;
}

void video_fade_out_init(uint8_t speed) {
	fadeType = FADE_OUT;
	fadeSpeed = speed;
}

void video_fade_in_skip_next() {
	skipFadeIn = TRUE;
}

int video_color_get_best_match(int r, int g, int b, PALETTE palette) {
	int color = 1;
	RGB target = (RGB) {.r = r, .g = g, .b = b};
	int distance = video_get_color_distance(&palette[color], &target);
	for (int i = 1; i < PALETTE_ENTRIES; i++) {
		int newDistance = video_get_color_distance(&palette[i], &target);
		if (newDistance < distance) {
			color = i;
			distance = newDistance;
		}
	}
	return color;
}

#include <allegro/gfx.h>
#include "video.h"

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

int video_load_raw_palette(const char *filename, PALETTE p) {
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

#ifndef TEXT_H
#define TEXT_H
#include <allegro/gfx.h>
#include <allegro/palette.h>
#include <allegro/font.h>
#include "common/common.h"

InitializationStatusEnum text_init_system(uint16_t numberOfTexts);
void text_free_all();
InitializationStatusEnum text_load_texts_from_file(const char * filename);
const char* text_get_by_id(uint16_t textId);
void text_out_multicolor(struct BITMAP *bmp, const struct FONT *f, const char *str, int x, int y, int color, int bg);
void text_out_multicolor_shadow(struct BITMAP *bmp, const struct FONT *f, const char *str, int x, int y, int color, int bg, int shadowColor);
void text_out_box(BITMAP *bmp, FONT *font, const char *str, int x, int y, int maxWidth, int maxHeight, int color, int bg);
void text_out_box_shadow(BITMAP *bmp, FONT *font, const char *str, int x, int y, int maxWidth, int maxHeight, int color, int bg, int shadowColor);

#endif /* TEXT_H */

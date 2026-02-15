#ifndef RENDER_QUEUE_H
#define RENDER_QUEUE_H
#include "common/common.h"
#include "common/text.h"
#include <allegro/palette.h>
#include <allegro/draw.h>
#include <allegro/font.h>
#include <allegro/text.h>
#include <allegro/rle.h>

typedef enum {
	RND_FLAG_NORMAL,
	RND_FLAG_H_FLIP,
	RND_FLAG_V_FLIP,
	RND_FLAG_HV_FLIP,
} RENDER_FLAGS;

typedef enum {
	RND_CMD_CLEAR,
	RND_CMD_SOLID,
	RND_CMD_SPRITE,
    RND_CMD_RLE_SPRITE,
	RND_CMD_MASKED_PARTIAL,
    RND_CMD_LINE,
	RND_CMD_RECT,
	RND_CMD_RECT_FILL,
	RND_CMD_TEXT,
    RND_CMD_ENCLOSED_TEXT,
	RND_CMD_TEXT_MULTICOLOR,
	RND_CMD_SOLID_PARTIAL
} RenderCommandType;

typedef struct {
    int color;
} RenderClearCommand;

typedef struct {
    BITMAP* bitmap;
    int x;
    int y;
} RenderSolidCommand;

typedef struct {
    BITMAP* bitmap;
    int originX, originY;    
    int destX, destY;
    int height, width;
} RenderSolidPartialCommand;

typedef struct {
    BITMAP* bitmap;
    int x;
    int y;
    int flags;
} RenderSpriteCommand;

typedef struct {
    RLE_SPRITE * rleSprite;
    int x;
    int y;
} RenderRleSpriteCommand;

typedef struct {
	BITMAP *bitmap;
	int originX, originY;
	int destX, destY;
	int height, width;
} RenderMaskedPartialCommand;

typedef struct {
    int x1, y1, x2, y2;
    int color;
} RenderRectFillCommand;

typedef struct {
    int x1, y1, x2, y2;
    int color;
} RenderLineCommand;

typedef struct {
    int x1, y1, x2, y2;
    int color;
} RenderRectCommand;

typedef struct {
    FONT *font;
    const char* text;
    int x, y;
    int color;
    int background;
    int shadowColor;
} RenderTextCommand;

typedef struct {
    FONT *font;
    const char* text;
    int x, y;
    int maxWidth, maxHeight;
    int color;
    int background;
    int shadowColor;
} RenderEnclosedTextCommand;

typedef struct {
    RenderCommandType type;
    int zOrder;
    union {
        RenderSpriteCommand sprite;
        RenderRleSpriteCommand rleSprite;
		RenderMaskedPartialCommand maskedPartial;
		RenderRectFillCommand rectFill;
        RenderLineCommand line;
		RenderRectCommand rect;
        RenderTextCommand text;
        RenderEnclosedTextCommand enclosedText;
        RenderClearCommand clear;
        RenderSolidCommand solid;
        RenderSolidPartialCommand solidPartial;
    } data;
} RenderCommand;

#define MAX_COMMANDS 2048

typedef struct {
    RenderCommand commands[MAX_COMMANDS];
    RenderCommand* sortedCommands[MAX_COMMANDS];
    int count;
} RenderQueue;

void render_queue_init(RenderQueue* queue);
void render_queue_clear(RenderQueue* queue);
void render_queue_submit_sprite(RenderQueue* queue, int z, BITMAP* bmp, int x, int y, int flags);
void render_queue_submit_rle_sprite(RenderQueue *queue, int z, RLE_SPRITE *rleSPrite, int x, int y);
void render_queue_submit_line(RenderQueue *queue, int z, int x1, int y1, int x2, int y2, int color);
void render_queue_submit_rect(RenderQueue *queue, int z, int x1, int y1, int x2, int y2, int color);
void render_queue_submit_rect_fill(RenderQueue* queue, int z, int x1, int y1, int x2, int y2, int color);
void render_queue_submit_clear(RenderQueue* queue, int z, int color);
void render_queue_submit_solid(RenderQueue* queue, int z, BITMAP* bmp, int x, int y);
void render_queue_submit_text(RenderQueue *queue, int z, FONT *font, const char *text, int x, int y, int color, int background);
void render_queue_submit_text_shadow(RenderQueue *queue, int z, FONT *font, const char *text, int x, int y, int color, int background, int shadowColor);
void render_queue_submit_enclosed_text(RenderQueue *queue, int z, FONT *font, const char *text, int x, int y, int maxWidth, int maxHeight, int color, int background);
void render_queue_submit_enclosed_text_shadow(RenderQueue *queue, int z, FONT *font, const char *text, int x, int y, int maxWidth, int maxHeight, int color, int background, int shadowColor);
void render_queue_submit_text_multicolor(RenderQueue *queue, int z, FONT *font, const char *text, int x, int y, int color, int background);
void render_queue_submit_text_multicolor_shadow(RenderQueue *queue, int z, FONT *font, const char *text, int x, int y, int color, int background, int shadowColor);
void render_queue_execute(RenderQueue* queue, BITMAP* targetBmp);
void render_queue_submit_solid_partial(RenderQueue *queue, int z, BITMAP *bmp, int originX, int originY, int destX, int destY, int width, int height);
void render_queue_submit_masked_partial(RenderQueue *queue, int z, BITMAP *bmp, int originX, int originY, int destX, int destY, int width, int height);

#endif /* RENDER_QUEUE_H */

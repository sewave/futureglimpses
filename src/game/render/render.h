#ifndef RENDER_H
#define RENDER_H
#include "../game_lib.h"

#define VIEWPORT_X_OFFSET 72
#define VIEWPORT_Y_OFFSET 12
#define VIEWPORT_WIDTH_TILES 15
#define VIEWPORT_HEIGHT_TILES 11
#define VIEWPORT_WIDTH VIEWPORT_WIDTH_TILES * TILE_SIZE
#define VIEWPORT_HEIGHT VIEWPORT_HEIGHT_TILES * TILE_SIZE
#define MAX_CAMERA_X_POSITION (WORLD_WIDTH - VIEWPORT_WIDTH)
#define MAX_CAMERA_Y_POSITION (WORLD_HEIGHT - VIEWPORT_HEIGHT)
#define VIEWPORT_X_MIN VIEWPORT_X_OFFSET
#define VIEWPORT_X_MAX VIEWPORT_X_OFFSET + VIEWPORT_WIDTH - 1
#define VIEWPORT_Y_MIN VIEWPORT_Y_OFFSET
#define VIEWPORT_Y_MAX VIEWPORT_Y_OFFSET + VIEWPORT_HEIGHT - 1

void render_queue_add_active_units(GameContext *context, RenderQueue *renderQueue);
void render_queue_add_active_objects(GameContext *context, RenderQueue *renderQueue);
void render_queue_submit_ui(GameContext *context, RenderQueue *renderQueue);

#endif /* RENDER_H */

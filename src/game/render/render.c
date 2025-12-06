#include "render.h"

void render_queue_units(GameContext* context, RenderQueue* renderQueue) {
    GameUnit* unit = context->units;
    uint16_t cameraMinX = context->xPosition / TILE_SIZE;
    uint16_t cameraMaxX = (context->xPosition + VIEWPORT_WIDTH) / TILE_SIZE;
    uint16_t cameraMinY = context->yPosition / TILE_SIZE;
    uint16_t cameraMaxY = (context->yPosition + VIEWPORT_HEIGHT) / TILE_SIZE;
    // TODO For now render all active units if on-screen
    for(int i = 0; i < MAX_GAME_UNITS; i++, unit++) {
        if(unit->isActive && unit->x >= cameraMinX && unit->x <= cameraMaxX && unit->y >= cameraMinY && unit->y <= cameraMaxY) {
            // TODO movement interpolation
            // TODO print selection box if unit is selected
            AnimationStatus* animationStatus = &unit->animationStatus;
            AnimationProperties* prop = animationStatus->animation->prop;
            int unitXCamera = (unit->x * TILE_SIZE) - context->xPosition - prop->xRepos + VIEWPORT_X_OFFSET;
            int unitYCamera = (unit->y * TILE_SIZE) - context->yPosition - prop->yRepos + VIEWPORT_Y_OFFSET;
            render_queue_submit_masked_partial(renderQueue, SPRITES_Z_ORDER + unit->y, animationStatus->sheet,
                animationStatus->animation->data->frames[animationStatus->frame].xOffset,
                prop->yOffset,
                unitXCamera,
                unitYCamera,
                prop->width,
                prop->height
            );
        }
    }
}
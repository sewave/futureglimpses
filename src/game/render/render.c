#include "render.h"
#include <allegro/color.h>

#define MOVE_PRECISION 16384
#define HEALTH_BAR_Y_OFFSET 6
#define HEALTH_BAR_HEIGHT 2
#define HEALTH_BAR_BORDER 1
#define HEALTH_BAR_QUARTER 4
#define HEALTH_BAR_HALF 2

void render_queue_units(GameContext *context, RenderQueue *renderQueue) {
	GameUnit *unit = context->units;
	uint16_t cameraMinX = context->xPosition / TILE_SIZE;
	uint16_t cameraMaxX = (context->xPosition + VIEWPORT_WIDTH) / TILE_SIZE;
	uint16_t cameraMinY = context->yPosition / TILE_SIZE;
	uint16_t cameraMaxY = (context->yPosition + VIEWPORT_HEIGHT) / TILE_SIZE;
	// TODO For now render all active units if on-screen
	for (int i = 0; i < MAX_GAME_UNITS; i++, unit++) {
		if (unit->isActive && unit->x >= cameraMinX && unit->x <= cameraMaxX && unit->y >= cameraMinY && unit->y <= cameraMaxY) {
			// TODO print selection box if unit is selected
			AnimationStatus *animationStatus = &unit->animationStatus;
			AnimationProperties *prop = animationStatus->animation->prop;
			int unitWorldX, unitWorldY;
			if (unit->state == UNIT_STATE_MOVE_ANIM) {
				int t = (unit->moveTimeCounter * MOVE_PRECISION) / unit->moveTime;
				t = clamp(t, 0, MOVE_PRECISION);

				int startX = unit->prevX * TILE_SIZE;
				int startY = unit->prevY * TILE_SIZE;
				int finalX = unit->x * TILE_SIZE;
				int finalY = unit->y * TILE_SIZE;
				unitWorldX = (startX * (MOVE_PRECISION - t)) / MOVE_PRECISION + (finalX * t) / MOVE_PRECISION;
				unitWorldY = (startY * (MOVE_PRECISION - t)) / MOVE_PRECISION + (finalY * t) / MOVE_PRECISION;
			} else {
				unitWorldX = (unit->x * TILE_SIZE);
				unitWorldY = (unit->y * TILE_SIZE);
			}

			int unitXCamera = unitWorldX - context->xPosition - prop->xRepos + VIEWPORT_X_OFFSET;
			int unitYCamera = unitWorldY - context->yPosition - prop->yRepos + VIEWPORT_Y_OFFSET;
			render_queue_submit_masked_partial(renderQueue, SPRITES_Z_ORDER + unit->y, animationStatus->sheet,
											   animationStatus->animation->data->frames[animationStatus->frame].xOffset,
											   prop->yOffset,
											   unitXCamera,
											   unitYCamera,
											   prop->width,
											   prop->height);
			if (unit->health < unit->maxHealth && unit->state != UNIT_STATE_DIE) {
				int healthBarColor = PAL_COLOR_GREEN;
				if (unit->health < unit->maxHealth / HEALTH_BAR_QUARTER) {
                    healthBarColor = PAL_COLOR_RED;
                }
				else {
					if (unit->health < unit->maxHealth / HEALTH_BAR_HALF) healthBarColor = PAL_COLOR_YELLOW;
				}

				int healthBarYInit = unitYCamera + prop->yRepos - HEALTH_BAR_Y_OFFSET;
				int healthBarYEnd = unitYCamera + prop->yRepos - HEALTH_BAR_Y_OFFSET + HEALTH_BAR_HEIGHT;
				int healthBarLength = ((int) unit->health * TILE_SIZE) / unit->maxHealth;

				render_queue_submit_rect_fill(
						renderQueue,
						UI_Z_ORDER + 1,
						unitXCamera + prop->xRepos + HEALTH_BAR_BORDER,
						healthBarYInit,
						unitXCamera + prop->xRepos + healthBarLength,
						healthBarYEnd,
						healthBarColor);
				render_queue_submit_rect_fill(
						renderQueue,
						UI_Z_ORDER,
						unitXCamera + prop->xRepos + healthBarLength,
						healthBarYInit,
						unitXCamera + prop->xRepos + TILE_SIZE - HEALTH_BAR_BORDER,
						healthBarYEnd,
						PAL_COLOR_BLACK);
			}
		}
	}
}
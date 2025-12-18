#include "render.h"
#include <allegro/color.h>
#include <allegro/keyboard.h>

#define MOVE_PRECISION 16384
#define HEALTH_BAR_Y_OFFSET 6
#define HEALTH_BAR_HEIGHT 1
#define HEALTH_BAR_BORDER 1
#define HEALTH_BAR_QUARTER 4
#define HEALTH_BAR_HALF 2

static char* getStateLetter(UnitStateEnum state) {
    switch (state) {
    case UNIT_STATE_IDLE:
        return "I";
    case UNIT_STATE_ATTACK:
        return "A";
    case UNIT_STATE_DEFEND:
        return "D";
    case UNIT_STATE_MOVE:
        return "M";
    case UNIT_STATE_MOVE_ANIM:
        return "a";
    case UNIT_STATE_MOVE_ATTACK:
        return "m";
    case UNIT_STATE_WORK:
        return "W";
    case UNIT_STATE_DIE:
        return "d";
    default:
        return "X";
    }
    
}

static char* getDirectionLetter(DirectionEnum dir) {
    switch (dir) {
    case DIRECTION_NORTH:
        return "N";
	case DIRECTION_EAST:
        return "E";
	case DIRECTION_SOUTH:
        return "S";
	case DIRECTION_WEST:
        return "W";
    default:
        return "X";
    }    
}

void render_queue_add_active_units(GameContext *context, RenderQueue *renderQueue) {
	uint16_t cameraMinX = context->xPosition / TILE_SIZE;
	uint16_t cameraMaxX = (context->xPosition + VIEWPORT_WIDTH) / TILE_SIZE;
	uint16_t cameraMinY = context->yPosition / TILE_SIZE;
	uint16_t cameraMaxY = (context->yPosition + VIEWPORT_HEIGHT) / TILE_SIZE;
	GameUnit **activeUnits = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeUnits++) {
		GameUnit *unit = *activeUnits;
		if (unit->x >= cameraMinX && unit->x <= cameraMaxX && unit->y >= cameraMinY && unit->y <= cameraMaxY) {
			AnimationStatus *animationStatus = &unit->animationStatus;
			AnimationProperties *prop = animationStatus->animation.prop;
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

            int unitTileXCamera = unitWorldX - context->xPosition + VIEWPORT_X_OFFSET;
            int unitTileYCamera = unitWorldY - context->yPosition + VIEWPORT_Y_OFFSET;
			int unitXCamera = unitTileXCamera - prop->xRepos;
			int unitYCamera = unitTileYCamera - prop->yRepos;
			render_queue_submit_masked_partial(renderQueue, SPRITES_Z_ORDER + unit->y, animationStatus->sheet,
											   animationStatus->animation.data->frames[animationStatus->frame].xOffset,
											   prop->yOffset,
											   unitXCamera,
											   unitYCamera,
											   prop->width,
											   prop->height);
            // Some debug stuff
            if(key[KEY_G]) {
                render_queue_submit_text(
                    renderQueue,
                    SPRITES_Z_ORDER + unit->y + 1,
                    context->gameFont,
                    getStateLetter(unit->state),
                    unitTileXCamera,
                    unitTileYCamera + TILE_SIZE / 2,
                    PAL_COLOR_WHITE, PAL_COLOR_BLACK
                );
                char* direction = getDirectionLetter(unit->direction);
                render_queue_submit_text(
                    renderQueue,
                    SPRITES_Z_ORDER + unit->y + 1,
                    context->gameFont,
                    direction,
                    unitTileXCamera + TILE_SIZE / 2,
                    unitTileYCamera + TILE_SIZE / 2,
                    PAL_COLOR_WHITE, PAL_COLOR_BLACK
                );
            }

            int rectColor = PAL_COLOR_TRANS;

            if(unit->isSelected && !unit->blinkTime) rectColor = PAL_COLOR_GREEN;
            if(unit->blinkTime && unit->blinkTime % BLINK_MOD < BLINK_FRAMES) {
                rectColor = (unit->controller == UNIT_CONTROLLER_AI) ? PAL_COLOR_RED : PAL_COLOR_GREEN;
            }
            if(rectColor != PAL_COLOR_TRANS) {
                render_queue_submit_rect(renderQueue,
                    BACKGROUND_Z_ORDER + 100,
                    unitTileXCamera, unitTileYCamera,
                    unitTileXCamera + TILE_SIZE, unitTileYCamera + TILE_SIZE,
                    rectColor);       
            }

			if (unit->health < unit->maxHealth && unit->state != UNIT_STATE_DIE) {
				int healthBarColor = PAL_COLOR_GREEN;
				if (unit->health < unit->maxHealth / HEALTH_BAR_QUARTER) {
                    healthBarColor = PAL_COLOR_RED;
                }
				else {
					if (unit->health < unit->maxHealth / HEALTH_BAR_HALF) healthBarColor = PAL_COLOR_YELLOW;
				}

				int healthBarYInit = unitTileYCamera - HEALTH_BAR_Y_OFFSET;
				int healthBarYEnd = unitTileYCamera - HEALTH_BAR_Y_OFFSET + HEALTH_BAR_HEIGHT;
				int healthBarLength = ((int) unit->health * TILE_SIZE) / unit->maxHealth;

				render_queue_submit_rect_fill(
						renderQueue,
						UI_Z_ORDER + 1,
						unitTileXCamera + HEALTH_BAR_BORDER,
						healthBarYInit,
						unitTileXCamera + healthBarLength,
						healthBarYEnd,
						healthBarColor);
				render_queue_submit_rect_fill(
						renderQueue,
						UI_Z_ORDER,
						unitTileXCamera + healthBarLength,
						healthBarYInit,
						unitTileXCamera + TILE_SIZE - HEALTH_BAR_BORDER,
						healthBarYEnd,
						PAL_COLOR_BLACK);
			}
		}
	}
}

void render_queue_add_active_objects(GameContext *context, RenderQueue *renderQueue) {
	uint16_t cameraMinX = context->xPosition;
	uint16_t cameraMaxX = (context->xPosition + VIEWPORT_WIDTH);
	uint16_t cameraMinY = context->yPosition;
	uint16_t cameraMaxY = (context->yPosition + VIEWPORT_HEIGHT);
	Object **activeObjects = context->activeObjects;
	for (int i = 0; i < context->activeObjectsCount; i++, activeObjects++) {
		Object *object = *activeObjects;
		if (object->x >= cameraMinX && object->x < cameraMaxX && object->y >= cameraMinY && object->y < cameraMaxY) {
			AnimationStatus *animationStatus = &object->animationStatus;
			AnimationProperties *prop = animationStatus->animation.prop;

            int unitTileXCamera = object->currentX - context->xPosition + VIEWPORT_X_OFFSET;
            int unitTileYCamera = object->currentY - context->yPosition + VIEWPORT_Y_OFFSET;
			int unitXCamera = unitTileXCamera - prop->xRepos;
			int unitYCamera = unitTileYCamera - prop->yRepos;
			render_queue_submit_masked_partial(renderQueue, OBJECTS_Z_ORDER + object->y / TILE_SIZE, animationStatus->sheet,
											   animationStatus->animation.data->frames[animationStatus->frame].xOffset,
											   prop->yOffset,
											   unitXCamera,
											   unitYCamera,
											   prop->width,
											   prop->height);
            
		}
	}
}

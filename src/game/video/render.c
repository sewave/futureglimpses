#include "game/video/render.h"
#include <allegro/color.h>
#include <allegro/keyboard.h>
#include <stdio.h>

#define MOVE_PRECISION 16384
#define HEALTH_BAR_Y_OFFSET 6
#define HEALTH_BAR_HEIGHT 1
#define HEALTH_BAR_BORDER 1
#define HEALTH_BAR_LENGTH TILE_SIZE

#define X_OFFSETS 3
#define Y_OFFSETS 3
#define ARROW_DATA 3
#define ARROW_DATA_F_OFF 0
#define ARROW_DATA_X_OFF 1
#define ARROW_DATA_Y_OFF 2
#define CURSOR_SIZE 16
#define SELECT_CUBE_OFF 1
#define SELECT_CUBE_SIZE 16

static char fpsText[16];
static char activeText[64];
static const int8_t cursorEdges[Y_OFFSETS][X_OFFSETS][ARROW_DATA] = {
		{{0, 0, 0}, {16, 0, 0}, {32, -14, 0}},
		{{112, 0, 0}, {0, 0, 0}, {48, -14, 0}},
		{{96, 0, -14}, {80, 0, -14}, {64, -14, -14}},
};

static char *getStateLetter(UnitStateEnum state) {
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
			return "m";
		case UNIT_STATE_MOVE_ATTACK:
			return "a";
		case UNIT_STATE_WORK:
			return "W";
		case UNIT_STATE_DIE:
			return "d";
		default:
			return "X";
	}
}

static char *getDirectionLetter(DirectionEnum dir) {
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
		int unitSize = unit->tileSize * TILE_SIZE;
		if (unit->y <= cameraMaxY && unit->x <= cameraMaxX &&
			unit->x >= cameraMinX - unitSize && unit->y >= cameraMinY - unitSize &&
			game_unit_is_visible(context, unit)) {
			AnimationStatus *animationStatus = &unit->animationStatus;
			AnimationProperties *prop = animationStatus->animation.prop;
			int unitWorldX, unitWorldY;
			if (unit->state == UNIT_STATE_MOVE_ANIM) {
				int t = (unit->moveTimeCounter * MOVE_PRECISION) / unit->moveTimeAnim;
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
			render_queue_submit_rle_sprite(renderQueue, SPRITES_Z_ORDER + unit->y,
											animationStatus->sheet->frames[prop->startFrame + animationStatus->frame],
											unitXCamera, unitYCamera);
			// Some debug stuff
			if (context->isDebugEnabled) {
				render_queue_submit_text(
						renderQueue,
						SPRITES_Z_ORDER + unit->y + 1,
						context->gameFont,
						getStateLetter(unit->state),
						unitTileXCamera,
						unitTileYCamera + TILE_SIZE / 2,
						PAL_COLOR_WHITE, PAL_COLOR_BLACK);
				char *direction = getDirectionLetter(unit->direction);
				render_queue_submit_text(
						renderQueue,
						SPRITES_Z_ORDER + unit->y + 1,
						context->gameFont,
						direction,
						unitTileXCamera + TILE_SIZE / 2,
						unitTileYCamera + TILE_SIZE / 2,
						PAL_COLOR_WHITE, PAL_COLOR_BLACK);
			}

			int rectColor = PAL_COLOR_TRANS;
			int rectZ = BACKGROUND_Z_ORDER + 100;

			if (unit->isSelected && !unit->blinkTime) {
				if (unit->controller == UNIT_CONTROLLER_AI) {
					rectColor = PAL_COLOR_RED;
				} else {
					rectColor = PAL_COLOR_GREEN;
				}
			} 
			if (unit->blinkTime && unit->blinkTime % BLINK_MOD < BLINK_FRAMES) {
				if (unit->controller == UNIT_CONTROLLER_AI) {
					rectColor = PAL_COLOR_RED;
					rectZ++;
				} else {
					rectColor = PAL_COLOR_GREEN;
				}
			}
			if (rectColor != PAL_COLOR_TRANS) {
				render_queue_submit_rect(renderQueue,
										 rectZ,
										 unitTileXCamera + SELECT_CUBE_OFF, unitTileYCamera + SELECT_CUBE_OFF,
										 unitTileXCamera + (SELECT_CUBE_SIZE * unit->tileSize) - 2 * SELECT_CUBE_OFF,
										 unitTileYCamera + (SELECT_CUBE_SIZE * unit->tileSize) - 2 * SELECT_CUBE_OFF,
										 rectColor);
			}

			if (unit->isBuilding && context->buildPlacing.showBuilding) {
				render_queue_submit_rect(renderQueue,
										 rectZ,
										 unitTileXCamera, unitTileYCamera,
										 unitTileXCamera + (SELECT_CUBE_SIZE * unit->tileSize) - 1,
										 unitTileYCamera + (SELECT_CUBE_SIZE * unit->tileSize) - 1,
										 PAL_COLOR_WHITE);
			}

			LifeBarEnum lifeBarSetting = context->config.lifeBar;
			uint8_t showHealthBar = unit->state != UNIT_STATE_DIE && ((lifeBarSetting != LIFE_BAR_NEVER &&
												   (lifeBarSetting == LIFE_BAR_ALWAYS || (lifeBarSetting == LIFE_BAR_DAMAGED && unit->health < unit->maxHealth))) ||
												  keyboard_is_key_down(KEY_ALT));
			if(unit->type == UNIT_TYPE_WORKER && unit->typed.workerData.carriedResourceQty > 0) {
				RLE_SPRITE* resourceIcon = game_gfx_get_unit_icon(GAME_ICON_GOLD + unit->typed.workerData.carriedResourceType);
				render_queue_submit_rle_sprite(renderQueue, UI_Z_ORDER + 1, resourceIcon,
						unitTileXCamera - showHealthBar * resourceIcon->w, unitTileYCamera - resourceIcon->h);
			}
			if (showHealthBar) {
				int healthBarColor = PAL_COLOR_DARK_GREEN;
				if (unit->health < unit->maxHealth / HEALTH_BAR_QUARTER) {
					healthBarColor = PAL_COLOR_RED;
				} else {
					if (unit->health < unit->maxHealth / HEALTH_BAR_HALF) healthBarColor = PAL_COLOR_YELLOW;
				}

				int healthBarYInit = unitTileYCamera - HEALTH_BAR_Y_OFFSET;
				int healthBarYEnd = unitTileYCamera - HEALTH_BAR_Y_OFFSET + HEALTH_BAR_HEIGHT;
				int healthBarLength = ((int) (unit->health * (HEALTH_BAR_LENGTH * unit->tileSize - 3 * HEALTH_BAR_BORDER))) / unit->maxHealth;

				render_queue_submit_rect_fill(
						renderQueue,
						UI_Z_ORDER + 1,
						unitTileXCamera + HEALTH_BAR_BORDER,
						healthBarYInit,
						unitTileXCamera + HEALTH_BAR_BORDER + healthBarLength,
						healthBarYEnd,
						healthBarColor);
				render_queue_submit_rect_fill(
						renderQueue,
						UI_Z_ORDER,
						unitTileXCamera + HEALTH_BAR_BORDER + healthBarLength,
						healthBarYInit,
						unitTileXCamera + HEALTH_BAR_BORDER + HEALTH_BAR_LENGTH * unit->tileSize - 3 * HEALTH_BAR_BORDER,
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
		if (object->currentX >= cameraMinX && object->currentX < cameraMaxX && object->currentY >= cameraMinY && object->currentY < cameraMaxY) {
			AnimationStatus *animationStatus = &object->animationStatus;
			AnimationProperties *prop = animationStatus->animation.prop;

			int unitTileXCamera = object->currentX - context->xPosition + VIEWPORT_X_OFFSET;
			int unitTileYCamera = object->currentY - context->yPosition + VIEWPORT_Y_OFFSET;
			int unitXCamera = unitTileXCamera - prop->xRepos;
			int unitYCamera = unitTileYCamera - prop->yRepos;
			render_queue_submit_rle_sprite(renderQueue, OBJECTS_Z_ORDER + object->currentY / TILE_SIZE,
											animationStatus->sheet->frames[prop->startFrame + animationStatus->frame],
											unitXCamera, unitYCamera);
		}
	}
}

void render_queue_submit_ui(GameContext *context, RenderQueue *renderQueue) {
	// Selected unit info
	render_queue_submit_solid_partial(renderQueue, BACKGROUND_Z_ORDER, context->renderedBoard,
									  context->xPosition, context->yPosition,
									  VIEWPORT_X_OFFSET, VIEWPORT_Y_OFFSET,
									  VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	// Submit to render the viewport from the renderedBoard
	BITMAP *frame = game_gfx_get_frame();
	render_queue_submit_sprite(renderQueue, UI_Z_ORDER + 500, frame, 0, 0, RND_FLAG_NORMAL);

	// Minimap
	render_queue_submit_solid(renderQueue, UI_Z_ORDER + 501, context->renderedMinimap,
							  MINIMAP_X_POS, MINIMAP_Y_POS);

	render_queue_submit_sprite(renderQueue, UI_Z_ORDER + 502, context->renderedMinimapUnits,
							   MINIMAP_X_POS, MINIMAP_Y_POS, RND_FLAG_NORMAL);

	// Minimap viewport window
	render_queue_submit_rect(renderQueue,
							 UI_Z_ORDER + 503,
							 context->xPosition / TILE_SIZE + MINIMAP_X_POS,
							 context->yPosition / TILE_SIZE + MINIMAP_Y_POS,
							 context->xPosition / TILE_SIZE + MINIMAP_X_POS + VIEWPORT_WIDTH_TILES - 1,
							 context->yPosition / TILE_SIZE + MINIMAP_Y_POS + VIEWPORT_HEIGHT_TILES - 1,
							 PAL_COLOR_WHITE);

	snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
	render_queue_submit_text(renderQueue, UI_Z_ORDER + 510, context->gameFont, fpsText, 260, 180, PAL_COLOR_WHITE, -1);

	if (context->isDebugEnabled) {
		int blue = 0;
		int red = 0;
		// Count active units by controller
		for (int i = 0; i < context->activeUnitCount; i++) {
			GameUnit *unit = context->activeUnits[i];
			if (unit->controller == UNIT_CONTROLLER_PLAYER) {
				blue++;
			} else {
				red++;
			}
		}
		snprintf(activeText, sizeof(activeText), "T:%d ^004B:%d ^005R:%d", context->activeUnitCount, blue, red);
		render_queue_submit_text_multicolor(renderQueue, UI_Z_ORDER + 510, context->gameFont, activeText,
			220, 2, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
	}

	resource_render_queue_submit_ui(context, renderQueue);

	// Target position blinkingg square
	if(context->targetPosition.x != NO_TARGET_POSITION && context->targetPosition.y != NO_TARGET_POSITION &&
		context->targetBlinkTime && context->targetBlinkTime % BLINK_MOD < BLINK_FRAMES) {
		uint16_t cameraMinX = context->xPosition / TILE_SIZE;
		uint16_t cameraMaxX = (context->xPosition + VIEWPORT_WIDTH) / TILE_SIZE;
		uint16_t cameraMinY = context->yPosition / TILE_SIZE;
		uint16_t cameraMaxY = (context->yPosition + VIEWPORT_HEIGHT) / TILE_SIZE;
		if (context->targetPosition.x >= cameraMinX && context->targetPosition.x < cameraMaxX &&
			context->targetPosition.y >= cameraMinY && context->targetPosition.y < cameraMaxY) {
			int unitTileXCamera = context->targetPosition.x * TILE_SIZE - context->xPosition + VIEWPORT_X_OFFSET;
			int unitTileYCamera = context->targetPosition.y * TILE_SIZE - context->yPosition + VIEWPORT_Y_OFFSET;	
			render_queue_submit_rect(renderQueue, OBJECTS_Z_ORDER, unitTileXCamera, unitTileYCamera, unitTileXCamera + TILE_SIZE,
				unitTileYCamera + TILE_SIZE, PAL_COLOR_GREEN);
		}
	}
}

void render_queue_submit_mouse(GameContext *context, RenderQueue *renderQueue) {
	switch (game_mouse_get_cursor_state()) {
		case MOUSE_CURSOR_SELECT:
			// Selection rectangle
			int selectionEndX = clamp(context->mouseStatus.x, VIEWPORT_X_MIN, VIEWPORT_X_MAX);
			int selectionEndY = clamp(context->mouseStatus.y, VIEWPORT_Y_MIN, VIEWPORT_Y_MAX);
			render_queue_submit_rect(renderQueue,
									 MOUSE_Z_ORDER,
									 game_mouse_get_selection_start_x(), game_mouse_get_selection_start_y(),
									 selectionEndX, selectionEndY,
									 PAL_COLOR_GREEN);
			break;
		default:
			uint8_t mouseEdgeX = 1;
			uint8_t mouseEdgeY = 1;
			if (context->mouseStatus.x < MOUSE_X_GO_LEFT) mouseEdgeX = 0;
			if (context->mouseStatus.x > MOUSE_X_GO_RIGHT) mouseEdgeX = 2;
			if (context->mouseStatus.y < MOUSE_Y_GO_UP) mouseEdgeY = 0;
			if (context->mouseStatus.y > MOUSE_Y_GO_DOWN) mouseEdgeY = 2;
			//If we are on the screen edges, we use the arrow sprites

			if ((mouseEdgeX == 1 && mouseEdgeY == 1) || context->gameState != GAME_STATE_PLAY_MAP) {
				// Mouse cursor
				render_queue_submit_sprite(renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(),
										   context->mouseStatus.x - mouse_x_focus, context->mouseStatus.y - mouse_y_focus,
										   RND_FLAG_NORMAL);
				if (context->buildPlacing.showBuilding && context->gameState == GAME_STATE_PLAY_MAP) {
					uint16_t framePos = game_animation_unit_get_frame_position(
							context->buildPlacing.building, UNIT_STATE_IDLE, DIRECTION_NORTH, 0);
					SpriteSheet *buildingSheet = game_gfx_get_unit_sheet(context->buildPlacing.building, UNIT_CONTROLLER_PLAYER);

					int mouseGridXOff = context->buildPlacing.x - context->xPosition / TILE_SIZE;
					int mouseGridYOff = context->buildPlacing.y - context->yPosition / TILE_SIZE;
					int buildX = mouseGridXOff * TILE_SIZE + VIEWPORT_X_OFFSET - context->xPosition % TILE_SIZE;
					int buildY = mouseGridYOff * TILE_SIZE + VIEWPORT_Y_OFFSET - context->yPosition % TILE_SIZE;

					render_queue_submit_rle_sprite(
							renderQueue, OBJECTS_Z_ORDER + 900, buildingSheet->frames[framePos],
							buildX, buildY);
					int quadX = buildX;
					BITMAP* buildOkBitmap = game_gfx_get_overtile(GAME_OVERTILE_OK);
					BITMAP* buildKoBitmap = game_gfx_get_overtile(GAME_OVERTILE_KO);
					for(int xOff = 0; xOff < context->buildPlacing.size; xOff++, quadX += TILE_SIZE) {
						int quadY = buildY;
						for(int yOff = 0; yOff < context->buildPlacing.size; yOff++) {
							BITMAP* overtile;
							if(context->buildPlacing.placeResult[xOff][yOff]) {
								overtile = buildOkBitmap;
							}
							else {
								overtile = buildKoBitmap;
							}
							render_queue_submit_sprite(renderQueue, UI_Z_ORDER + 1, overtile,
								quadX, quadY, RND_FLAG_NORMAL);
							quadY += TILE_SIZE;
						}
					}
				}
			} else {
				// Arrow cursor
				render_queue_submit_masked_partial(
						renderQueue, MOUSE_Z_ORDER, game_mouse_get_arrow_cursors_sheet(),
						cursorEdges[mouseEdgeY][mouseEdgeX][ARROW_DATA_F_OFF], 0,
						context->mouseStatus.x + cursorEdges[mouseEdgeY][mouseEdgeX][ARROW_DATA_X_OFF],
						context->mouseStatus.y + cursorEdges[mouseEdgeY][mouseEdgeX][ARROW_DATA_Y_OFF],
						CURSOR_SIZE, CURSOR_SIZE);
			}
			break;
	}
}

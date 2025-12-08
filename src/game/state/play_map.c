#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define MINIMAP_X_POS 3
#define MINIMAP_Y_POS 6

#define MOUSE_X_GO_LEFT TILE_SIZE / 2
#define MOUSE_X_GO_RIGHT (GAME_INTERNAL_WIDTH - TILE_SIZE / 2)
#define MOUSE_Y_GO_UP TILE_SIZE / 2
#define MOUSE_Y_GO_DOWN (GAME_INTERNAL_HEIGHT - TILE_SIZE / 2)

int selectionStartX = -1;
int selectionStartY = -1;

static char fpsText[16];
int moveViewportCounter = 0;

void handle_mouse_selection(GameContext *context, RenderQueue *renderQueue) {
	int mouseX = mouse_get_x();
	int mouseY = mouse_get_y();
	
	if (mouseY > VIEWPORT_Y_MIN && mouseY < VIEWPORT_Y_MAX &&
		mouseX > VIEWPORT_X_MIN && mouseX < VIEWPORT_X_MAX &&
		context->mouse.isLeftDown && !context->mouse.wasLeftDown) {
		selectionStartX = mouseX;
		selectionStartY = mouseY;
		context->mouse.isSelecting = TRUE;
	}

	if (!context->mouse.isLeftDown && context->mouse.wasLeftDown && context->mouse.isSelecting) {
		context->mouse.isSelecting = FALSE;

		int selectionEndX = mouseX;
		int selectionEndY = mouseY;

		int dx = abs(selectionEndX - selectionStartX);
		int dy = abs(selectionEndY - selectionStartY);

		for (int i = 0; i < context->selectedUnitCount; i++) {
			GameUnit* unit = game_unit_get_by_id(context, context->selectedUnits[i]);
			if (unit) {
				unit->isSelected = FALSE;
			}
		}
		context->selectedUnitCount = 0;

		if (dx < TILE_SIZE && dy < TILE_SIZE) {
			// Simple Click unitary
			int worldX = context->xPosition + selectionStartX;
			int worldY = context->yPosition + selectionStartY;
			int tileX = worldX / TILE_SIZE;
			int tileY = worldY / TILE_SIZE;

			if (tileX >= 0 && tileX < BOARD_WIDTH && tileY >= 0 && tileY < BOARD_HEIGHT) {
				int id = context->walkabilityGrid[tileX][tileY];

				GameUnit *u = game_unit_get_by_id(context, id);

				if (u) {
					context->selectedUnits[0] = id;
					context->selectedUnitCount = 1;
					u->isSelected = TRUE;
				}
			}
		} else {
			// Mass Selection (Bounding Box)
			int minScreenX = min_val(selectionStartX, selectionEndX) - VIEWPORT_X_OFFSET;
			int maxScreenX = max_val(selectionStartX, selectionEndX) - VIEWPORT_X_OFFSET;
			int minScreenY = min_val(selectionStartY, selectionEndY) - VIEWPORT_Y_OFFSET;
			int maxScreenY = max_val(selectionStartY, selectionEndY) - VIEWPORT_Y_OFFSET;

			int worldBoxMinX = context->xPosition + minScreenX;
			int worldBoxMaxX = context->xPosition + maxScreenX;
			int worldBoxMinY = context->yPosition + minScreenY;
			int worldBoxMaxY = context->yPosition + maxScreenY;

			int tileMinX = worldBoxMinX / TILE_SIZE;
			int tileMaxX = worldBoxMaxX / TILE_SIZE;
			int tileMinY = worldBoxMinY / TILE_SIZE;
			int tileMaxY = worldBoxMaxY / TILE_SIZE;

			tileMinX = clamp(tileMinX, 0, BOARD_WIDTH - 1);
			tileMaxX = clamp(tileMaxX, 0, BOARD_WIDTH - 1);
			tileMinY = clamp(tileMinY, 0, BOARD_HEIGHT - 1);
			tileMaxY = clamp(tileMaxY, 0, BOARD_HEIGHT - 1);
			printf("Selecting in (%d-%d)-(%d-%d)\n", tileMinX, tileMinY, tileMaxX, tileMaxY);

			unsigned char alreadySelected[MAX_GAME_UNITS];
			memset(alreadySelected, 0, sizeof(alreadySelected));

			for (int row = tileMinY; row <= tileMaxY; row++) {
				for (int col = tileMinX; col <= tileMaxX; col++) {
					UnitId id = context->walkabilityGrid[col][row];
					if (id < HANDLE_ID_THRESHOLD) continue;
					GameUnit *u = game_unit_get_by_id(context, id);
					if (u) {
						int entityIndex = GET_INDEX(id);
						if (u->controller == UNIT_CONTROLLER_PLAYER && !alreadySelected[entityIndex]) {
							if (context->selectedUnitCount < MAX_GAME_UNITS) {
								context->selectedUnits[context->selectedUnitCount++] = id;
								u->isSelected = TRUE;
								alreadySelected[entityIndex] = TRUE;
							}
						}
					}
				}
			}
			printf("%d units selected.\n", context->selectedUnitCount);
		}
	}
}

GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue) {
	// TODO change cursor style based on position UP-LEFT, UP-RIGHT, DOWN-LEFT, DOWN-RIGHT, LEFT, RIGHT, UP, DOWN
	int mouseX = mouse_get_x();
	int mouseY = mouse_get_y();
	context->mouse.isRightDown = mouse_b & 2;
	context->mouse.isLeftDown = mouse_b & 1;

	handle_mouse_selection(context, renderQueue);

	// If we click the mouse on the minimap, we should move the camera there
	if (context->mouse.isLeftDown & !context->mouse.isSelecting) {
		if (mouseX >= MINIMAP_X_POS &&
			mouseX <= MINIMAP_X_POS + BOARD_WIDTH &&
			mouseY >= MINIMAP_Y_POS &&
			mouseY <= MINIMAP_Y_POS + BOARD_HEIGHT) {
			context->xPosition = (mouseX - MINIMAP_X_POS - 8) * TILE_SIZE;
			context->yPosition = (mouseY - MINIMAP_Y_POS - 6) * TILE_SIZE;
			if (context->yPosition < 0) context->yPosition = 0;
			if (context->xPosition < 0) context->xPosition = 0;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
	}

	if ((key[KEY_UP] || key[KEY_DOWN] || key[KEY_LEFT] || key[KEY_RIGHT] ||
		 mouseX < MOUSE_X_GO_LEFT || mouseX > MOUSE_X_GO_RIGHT ||
		 mouseY < MOUSE_Y_GO_UP || mouseY > MOUSE_Y_GO_DOWN) &&
		!(context->mouse.isLeftDown)) {
		moveViewportCounter++;
	} else {
		moveViewportCounter = 0;
	}
	int cameraSpeed = 2;

	if (moveViewportCounter >= 1) {
		if (key[KEY_UP] || mouseY < MOUSE_Y_GO_UP) {
			context->yPosition -= cameraSpeed;
			if (context->yPosition < 0) context->yPosition = 0;
		}
		if (key[KEY_DOWN] || mouseY > MOUSE_Y_GO_DOWN) {
			context->yPosition += cameraSpeed;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
		if (key[KEY_LEFT] || mouseX < MOUSE_X_GO_LEFT) {
			context->xPosition -= cameraSpeed;
			if (context->xPosition < 0) context->xPosition = 0;
		}
		if (key[KEY_RIGHT] || mouseX > MOUSE_X_GO_RIGHT) {
			context->xPosition += cameraSpeed;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
		}
		moveViewportCounter = 0;
	}

	// TODO not each frame
	clear_bitmap(context->renderedMinimapUnits);

	// Process active units
	GameUnit **activeUnits = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeUnits++) {
		GameUnit *unit = *activeUnits;
		game_animation_unit_advance(context, unit);
		game_unit_ai_invoke(context, unit);
		int color = unit->controller == UNIT_CONTROLLER_PLAYER ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		// TODO buildings size
		putpixel(context->renderedMinimapUnits, unit->x, unit->y, color);
	}

	context->mouse.wasLeftDown = context->mouse.isLeftDown;
	context->mouse.wasRightDown = context->mouse.isRightDown;

	// If there are more ticks to draw, skip queue phase
	if (context->ticksToCatchup) return GAME_STATE_PLAY_MAP;

	render_queue_add_active_units(context, renderQueue);
	// TODO queue effects, proyectiles, particles, etc

	// Submit to render the viewport from the renderedBoard
	render_queue_submit_masked_partial(renderQueue, UI_Z_ORDER + 500, context->gameBack, 0, 0, 0, 0, 320, 200);

	render_queue_submit_solid_partial(renderQueue, BACKGROUND_Z_ORDER, context->renderedBoard,
									  context->xPosition, context->yPosition,
									  VIEWPORT_X_OFFSET, VIEWPORT_Y_OFFSET,
									  VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
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

	// Selection rectangle
	if (context->mouse.isSelecting) {
		int selectionEndX = clamp(mouse_get_x(), VIEWPORT_X_MIN, VIEWPORT_X_MAX);
		int selectionEndY = clamp(mouse_get_y(), VIEWPORT_Y_MIN, VIEWPORT_Y_MAX);
		render_queue_submit_rect(renderQueue,
							MOUSE_Z_ORDER,
							selectionStartX, selectionStartY, selectionEndX, selectionEndY,
							PAL_COLOR_GREEN);
	} else {
		// Mouse cursor
		render_queue_submit_sprite(renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(), mouse_get_x() - mouse_x_focus, mouse_get_y() - mouse_y_focus, RND_FLAG_NORMAL);
	}

	snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
	render_queue_submit_text(renderQueue, UI_Z_ORDER + 510, font, fpsText, 0, 190, PAL_COLOR_WHITE, -1);

	return GAME_STATE_PLAY_MAP;
}

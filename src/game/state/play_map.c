#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define MINIMAP_X_POS 4
#define MINIMAP_Y_POS 12

#define MOUSE_X_GO_LEFT TILE_SIZE / 2
#define MOUSE_X_GO_RIGHT (GAME_INTERNAL_WIDTH - TILE_SIZE / 2)
#define MOUSE_Y_GO_UP TILE_SIZE / 2
#define MOUSE_Y_GO_DOWN (GAME_INTERNAL_HEIGHT - TILE_SIZE / 2)

int selectionStartX = -1;
int selectionStartY = -1;

static char fpsText[16];
int moveViewportCounter = 0;

typedef enum {
	SELECTION_SET,
	SELECTION_ADD,
	SELECTION_REMOVE
} SelectionModeEnum;

static void clear_all_selections(GameContext *context) {
	for (int i = 0; i < context->selectedUnitCount; i++) {
		GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
		if (unit) unit->isSelected = FALSE;
	}
	context->selectedUnitCount = 0;
}

static void remove_unit_from_selection(GameContext *context, GameUnit *unit) {
	if (!unit->isSelected) return;
	unit->isSelected = FALSE;
	for (int i = 0; i < context->selectedUnitCount; i++) {
		if (context->selectedUnits[i] == unit->id) {
			// Remove from selection, move last selected index to removed position directly
			context->selectedUnits[i] = context->selectedUnits[--context->selectedUnitCount];
			return;
		}
	}
}

static void add_unit_to_selection(GameContext *context, GameUnit *unit) {
	if (unit->isSelected) return;
	unit->isSelected = TRUE;
	context->selectedUnits[context->selectedUnitCount++] = unit->id;
}

// We will search for target on that position or a unit that was there with previous positions
static UnitId get_in_position_or_previous(GameContext *context, int boardXPosition, int boardYPosition) {
	UnitId target = context->walkabilityGrid[boardXPosition][boardYPosition];
	if (target == WALKABILITY_FREE) {
		// Search on the sorrounding area for a unit that was there previously
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				if (x == 0 && y == 0) continue;
				int checkX = boardXPosition + x;
				int checkY = boardYPosition + y;
				if (checkX < BOARD_X_MIN || checkX > BOARD_X_MAX ||
					checkY < BOARD_Y_MIN || checkY > BOARD_Y_MAX) continue;
				UnitId checkId = context->walkabilityGrid[checkX][checkY];
				if (checkId < HANDLE_ID_THRESHOLD) continue;
				GameUnit *checkUnit = game_unit_get_by_id(context, checkId);
				if (!checkUnit) continue;
				// If the unit was previously on the target position, we consider it the target
				if (checkUnit->prevX == boardXPosition && checkUnit->prevY == boardYPosition) {
					target = checkId;
					break;
				}
			}
			if (target != WALKABILITY_FREE) break;
		}
	}
	return target;
}

void handle_units_area(GameContext *context, RenderQueue *renderQueue) {
	int mouseX = mouse_get_x();
	int mouseY = mouse_get_y();

	// Actions that must be inside the area

	// TODO handle autoactions, stop, defend

	if (mouseY > VIEWPORT_Y_MIN && mouseY < VIEWPORT_Y_MAX &&
		mouseX > VIEWPORT_X_MIN && mouseX < VIEWPORT_X_MAX) {

		if (context->mouse.isLeftDown && !context->mouse.wasLeftDown) {
			selectionStartX = mouseX;
			selectionStartY = mouseY;
			context->mouse.isSelecting = TRUE;
		}

		if (!context->mouse.isRightDown && context->mouse.wasRightDown) {
			// Contextual action

			// Get board situation
			int boardXPosition = (context->xPosition + mouseX - VIEWPORT_X_OFFSET) / TILE_SIZE;
			int boardYPosition = (context->yPosition + mouseY - VIEWPORT_Y_OFFSET) / TILE_SIZE;
			boardXPosition = clamp(boardXPosition, BOARD_X_MIN, BOARD_X_MAX);
			boardYPosition = clamp(boardYPosition, BOARD_Y_MIN, BOARD_Y_MAX);

			UnitId target = get_in_position_or_previous(context, boardXPosition, boardYPosition);

			if (target < HANDLE_ID_THRESHOLD) {
				// Not unit position, depending on the tile, we move or interact with resource
				for (int i = 0; i < context->selectedUnitCount; i++) {
					GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
					if (!unit) continue;
					if (target == WALKABILITY_FREE) {
						game_unit_command_move(unit, NULL, boardXPosition, boardYPosition);
					} else {
						// TODO Resource => Work
					}
				}
			} else {
				GameUnit *targetUnit = game_unit_get_by_id(context, target);
				if (targetUnit) {
					targetUnit->blinkTime = BLINK_TIME;

					for (int i = 0; i < context->selectedUnitCount; i++) {
						GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
						if (!unit) continue;
						if (targetUnit->controller == UNIT_CONTROLLER_AI) {
							game_unit_command_move_attack(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
						} else {
							game_unit_command_move(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
						}
					}
				}
			}
		}
	}

	// Releasing actions that can be outside the area
	if (!context->mouse.isLeftDown && context->mouse.wasLeftDown && context->mouse.isSelecting) {
		context->mouse.isSelecting = FALSE;

		int selectionEndX = mouseX;
		int selectionEndY = mouseY;

		int dx = abs(selectionEndX - selectionStartX);
		int dy = abs(selectionEndY - selectionStartY);

		SelectionModeEnum selectionMode = SELECTION_SET;
		if (key[KEY_LSHIFT] || key[KEY_RSHIFT]) {
			selectionMode = SELECTION_ADD;
		} else if (key[KEY_LCONTROL] || key[KEY_RCONTROL]) {
			selectionMode = SELECTION_REMOVE;
		}
		// TODO attack mode, move mode or selection mode

		if (dx < TILE_SIZE && dy < TILE_SIZE) {
			// Simple Click unitary
			int worldX = context->xPosition + selectionStartX - VIEWPORT_X_OFFSET;
			int worldY = context->yPosition + selectionStartY - VIEWPORT_Y_OFFSET;
			int tileX = worldX / TILE_SIZE;
			int tileY = worldY / TILE_SIZE;

			if (tileX >= BOARD_X_MIN && tileX <= BOARD_X_MAX && tileY >= BOARD_Y_MIN && tileY <= BOARD_Y_MAX) {
				UnitId id = get_in_position_or_previous(context, tileX, tileY);

				GameUnit *foundUnit = game_unit_get_by_id(context, id);

				if (foundUnit && foundUnit->controller == UNIT_CONTROLLER_PLAYER) {
					switch (selectionMode) {
						case SELECTION_SET:
							clear_all_selections(context);
							add_unit_to_selection(context, foundUnit);
							break;
						case SELECTION_ADD:
							add_unit_to_selection(context, foundUnit);
							break;
						case SELECTION_REMOVE:
							remove_unit_from_selection(context, foundUnit);
							break;
					}
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

			int tileMinX = clamp(worldBoxMinX / TILE_SIZE, BOARD_X_MIN, BOARD_X_MAX);
			int tileMaxX = clamp(worldBoxMaxX / TILE_SIZE, BOARD_X_MIN, BOARD_X_MAX);
			int tileMinY = clamp(worldBoxMinY / TILE_SIZE, BOARD_Y_MIN, BOARD_Y_MAX);
			int tileMaxY = clamp(worldBoxMaxY / TILE_SIZE, BOARD_Y_MIN, BOARD_Y_MAX);

			if(selectionMode == SELECTION_SET) clear_all_selections(context);

			for (int row = tileMinY; row <= tileMaxY; row++) {
				for (int col = tileMinX; col <= tileMaxX; col++) {
					UnitId id = context->walkabilityGrid[col][row];
					if (id < HANDLE_ID_THRESHOLD) continue;
					GameUnit *foundUnit = game_unit_get_by_id(context, id);
					if (foundUnit && foundUnit->controller == UNIT_CONTROLLER_PLAYER) {
						if (selectionMode == SELECTION_REMOVE) {
							remove_unit_from_selection(context, foundUnit);
						} else {
							add_unit_to_selection(context, foundUnit);
						}
					}
				}
			}
		}
	}
}

GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue) {
	// TODO change cursor style based on position UP-LEFT, UP-RIGHT, DOWN-LEFT, DOWN-RIGHT, LEFT, RIGHT, UP, DOWN
	int mouseX = mouse_get_x();
	int mouseY = mouse_get_y();
	context->mouse.isRightDown = mouse_b & 2;
	context->mouse.isLeftDown = mouse_b & 1;

	handle_units_area(context, renderQueue);

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
		(!context->mouse.isLeftDown || context->mouse.isSelecting)) {
		moveViewportCounter++;
	} else {
		moveViewportCounter = 0;
	}
	int cameraSpeed = 4;

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

	render_queue_submit_solid_partial(renderQueue, BACKGROUND_Z_ORDER, context->renderedBoard,
									  context->xPosition, context->yPosition,
									  VIEWPORT_X_OFFSET, VIEWPORT_Y_OFFSET,
									  VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	// Submit to render the viewport from the renderedBoard
	render_queue_submit_masked_partial(renderQueue, UI_Z_ORDER + 500, context->gameBack, 0, 0, 0, 0,
									   context->gameBack->w, context->gameBack->h);

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

	if (context->mouse.isSelecting) {
		// Selection rectangle
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

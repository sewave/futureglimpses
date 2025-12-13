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

static uint16_t lastSelectionSlotSelectedTime = 0;
static const uint16_t selectionSlotCooldown = SEC_TO_FRAMES(0.3);

static void start_slot_selection_time() {
	lastSelectionSlotSelectedTime = 1;
}

static void update_slot_selection_time() {
	if (lastSelectionSlotSelectedTime) {
		lastSelectionSlotSelectedTime++;
		if (lastSelectionSlotSelectedTime > selectionSlotCooldown) {
			lastSelectionSlotSelectedTime = 0;
		}
	}
}

static uint16_t get_board_x_position(uint16_t cameraPosition, int cursorPosition) {
	return clamp((cameraPosition + cursorPosition - VIEWPORT_X_OFFSET) / TILE_SIZE, BOARD_X_MIN, BOARD_X_MAX);
}

static uint16_t get_board_y_position(uint16_t cameraPosition, int cursorPosition) {
	return clamp((cameraPosition + cursorPosition - VIEWPORT_Y_OFFSET) / TILE_SIZE, BOARD_Y_MIN, BOARD_Y_MAX);
}

void handle_units_area(GameContext *context, RenderQueue *renderQueue) {
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;

	// Actions that must be inside the area
	// TODO handle autoactions, stop, defend
	if (mouseY > VIEWPORT_Y_MIN && mouseY < VIEWPORT_Y_MAX &&
		mouseX > VIEWPORT_X_MIN && mouseX < VIEWPORT_X_MAX) {

		if (context->mouseStatus.isLeftPressed) {
			selectionStartX = mouseX;
			selectionStartY = mouseY;
			context->mouseStatus.isSelecting = TRUE;
		}

		if (context->mouseStatus.isLeftDoubleClick) {
			// If we double click, select all units of the same type on screen
			int tileX = get_board_x_position(context->xPosition, mouseX);
			int tileY = get_board_y_position(context->yPosition, mouseY);
			UnitId id = game_selection_get_in_position_or_previous(context, tileX, tileY);
			GameUnit *sourceUnit = game_unit_get_by_id(context, id);
			if (sourceUnit && sourceUnit->controller == UNIT_CONTROLLER_PLAYER) {
				UnitTypeEnum targetType = sourceUnit->type;
				game_selection_clear(context);
				int tileMinX = clamp(context->xPosition / TILE_SIZE, BOARD_X_MIN, BOARD_X_MAX);
				int tileMaxX = tileMinX + VIEWPORT_WIDTH_TILES;
				int tileMinY = clamp(context->yPosition / TILE_SIZE, BOARD_Y_MIN, BOARD_Y_MAX);
				int tileMaxY = tileMinY + VIEWPORT_HEIGHT_TILES;
				for (int row = tileMinY; row <= tileMaxY; row++) {
					for (int col = tileMinX; col <= tileMaxX; col++) {
						UnitId id = context->walkabilityGrid[col][row];
						if (id < HANDLE_ID_THRESHOLD) continue;
						GameUnit *foundUnit = game_unit_get_by_id(context, id);
						if (foundUnit && foundUnit->controller == UNIT_CONTROLLER_PLAYER && foundUnit->type == targetType) {
							game_selection_add_unit(context, foundUnit);
						}
					}
				}
			}
			context->mouseStatus.isSelecting = FALSE;
		}

		if (context->mouseStatus.isRightPressed) {
			// TODO spawn mouse confirmation
			// Contextual action

			// Get board situation
			int boardXPosition = get_board_x_position(context->xPosition, mouseX);
			int boardYPosition = get_board_y_position(context->yPosition, mouseY);

			UnitId target = game_selection_get_in_position_or_previous(context, boardXPosition, boardYPosition);

			if (target < HANDLE_ID_THRESHOLD) {
				// Not unit position, depending on the tile, we move or interact with resource
				for (int i = 0; i < context->selectedUnitCount; i++) {
					GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
					if (!unit) continue;
					if (target == WALKABILITY_FREE) {
						game_unit_command_move(unit, NULL, boardXPosition, boardYPosition);
					} else {
						// TODO Resource => Work, if we are workers
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

	if (!context->mouseStatus.isLeftDoubleClick) {
		if (context->mouseStatus.isLeftReleased && context->mouseStatus.isSelecting) {
			context->mouseStatus.isSelecting = FALSE;

			int selectionEndX = mouseX;
			int selectionEndY = mouseY;

			int dx = abs(selectionEndX - selectionStartX);
			int dy = abs(selectionEndY - selectionStartY);

			SelectionModeEnum selectionMode = SELECTION_SET;
			if (keyboard_is_key_down(KEY_LSHIFT) || keyboard_is_key_down(KEY_RSHIFT)) {
				selectionMode = SELECTION_ADD;
			} else {
				if (keyboard_is_key_down(KEY_LCONTROL) || keyboard_is_key_down(KEY_RCONTROL)) {
					selectionMode = SELECTION_REMOVE;
				}
			}
			// TODO attack mode, move mode or selection mode

			if (dx < TILE_SIZE && dy < TILE_SIZE) {
				// Simple Click unitary
				int tileX = get_board_x_position(context->xPosition, selectionStartX);
				int tileY = get_board_y_position(context->yPosition, selectionStartY);

				if (tileX >= BOARD_X_MIN && tileX <= BOARD_X_MAX && tileY >= BOARD_Y_MIN && tileY <= BOARD_Y_MAX) {
					UnitId id = game_selection_get_in_position_or_previous(context, tileX, tileY);
					GameUnit *foundUnit = game_unit_get_by_id(context, id);
					if (foundUnit && foundUnit->controller == UNIT_CONTROLLER_PLAYER) {
						switch (selectionMode) {
							case SELECTION_SET:
								game_selection_clear(context);
								game_selection_add_unit(context, foundUnit);
								break;
							case SELECTION_ADD:
								game_selection_add_unit(context, foundUnit);
								break;
							case SELECTION_REMOVE:
								game_selection_remove_unit(context, foundUnit);
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

				if (selectionMode == SELECTION_SET) game_selection_clear(context);

				for (int row = tileMinY; row <= tileMaxY; row++) {
					for (int col = tileMinX; col <= tileMaxX; col++) {
						UnitId id = context->walkabilityGrid[col][row];
						if (id < HANDLE_ID_THRESHOLD) continue;
						GameUnit *foundUnit = game_unit_get_by_id(context, id);
						if (foundUnit && foundUnit->controller == UNIT_CONTROLLER_PLAYER) {
							if (selectionMode == SELECTION_REMOVE) {
								game_selection_remove_unit(context, foundUnit);
							} else {
								game_selection_add_unit(context, foundUnit);
							}
						}
					}
				}
			}
		}
	}
}


GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue) {
	// TODO menus
	if (keyboard_is_key_down(KEY_ESC)) return GAME_STATE_EXIT;

	// TODO change cursor style based on position UP-LEFT, UP-RIGHT, DOWN-LEFT, DOWN-RIGHT, LEFT, RIGHT, UP, DOWN
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;

	if (keyboard_is_key_pressed(KEY_SPACE)) {
		game_selection_center_camera_on_selection(context);
	}

	update_slot_selection_time();
	if (keyboard_is_key_down(KEY_LCONTROL) || keyboard_is_key_down(KEY_RCONTROL)) {
		if (keyboard_is_key_pressed(KEY_1)) game_selection_save_to_slot(context, SELECTION_SLOT_1);
		if (keyboard_is_key_pressed(KEY_2)) game_selection_save_to_slot(context, SELECTION_SLOT_2);
		if (keyboard_is_key_pressed(KEY_3)) game_selection_save_to_slot(context, SELECTION_SLOT_3);
		if (keyboard_is_key_pressed(KEY_4)) game_selection_save_to_slot(context, SELECTION_SLOT_4);
		if (keyboard_is_key_pressed(KEY_5)) game_selection_save_to_slot(context, SELECTION_SLOT_5);
	} else {
		uint16_t previousSlotTime = lastSelectionSlotSelectedTime;
		if (keyboard_is_key_pressed(KEY_1)) {
			game_selection_load_from_slot(context, SELECTION_SLOT_1);
			start_slot_selection_time();
		}
		if (keyboard_is_key_pressed(KEY_2)) {
			game_selection_load_from_slot(context, SELECTION_SLOT_2);
			start_slot_selection_time();
		}
		if (keyboard_is_key_pressed(KEY_3)) {
			game_selection_load_from_slot(context, SELECTION_SLOT_3);
			start_slot_selection_time();
		}
		if (keyboard_is_key_pressed(KEY_4)) {
			game_selection_load_from_slot(context, SELECTION_SLOT_4);
			start_slot_selection_time();
		}
		if (keyboard_is_key_pressed(KEY_5)) {
			game_selection_load_from_slot(context, SELECTION_SLOT_5);
			start_slot_selection_time();
		}
		if (previousSlotTime > lastSelectionSlotSelectedTime && lastSelectionSlotSelectedTime == 1) {
			game_selection_center_camera_on_selection(context);
		}
	}

	handle_units_area(context, renderQueue);

	// If we click the mouseStatus on the minimap, we should move the camera there
	if (mouseX >= MINIMAP_X_POS &&
		mouseX <= MINIMAP_X_POS + BOARD_WIDTH &&
		mouseY >= MINIMAP_Y_POS &&
		mouseY <= MINIMAP_Y_POS + BOARD_HEIGHT) {
		if (context->mouseStatus.isLeftDown & !context->mouseStatus.isSelecting) {
			// TODO move and attack modes from minimap
			context->xPosition = (mouseX - MINIMAP_X_POS - 8) * TILE_SIZE;
			context->yPosition = (mouseY - MINIMAP_Y_POS - 6) * TILE_SIZE;
			if (context->yPosition < 0) context->yPosition = 0;
			if (context->xPosition < 0) context->xPosition = 0;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
	}

	if ((key[KEY_UP] || key[KEY_DOWN] || key[KEY_LEFT] || key[KEY_RIGHT]) ||
		((mouseX < MOUSE_X_GO_LEFT || mouseX > MOUSE_X_GO_RIGHT ||
		  mouseY < MOUSE_Y_GO_UP || mouseY > MOUSE_Y_GO_DOWN) &&
		 !context->mouseStatus.isLeftDown)) {
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

	// TODO not each frame?
	clear_bitmap(context->renderedMinimapUnits);

	// Process active units
	GameUnit **activeUnits = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeUnits++) {
		GameUnit *unit = *activeUnits;
		game_animation_unit_advance(context, unit);
		game_unit_ai_invoke(context, unit);
		int color = unit->controller == UNIT_CONTROLLER_PLAYER ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		if (unit->tileSize == 1) {
			putpixel(context->renderedMinimapUnits, unit->x, unit->y, color);
		} else {
			for (int dx = 0; dx < unit->tileSize; dx++) {
				for (int dy = 0; dy < unit->tileSize; dy++) {
					putpixel(context->renderedMinimapUnits, unit->x + dx, unit->y + dy, color);
				}
			}
		}
	}

	context->mouseStatus.wasLeftDown = context->mouseStatus.isLeftDown;
	context->mouseStatus.wasRightDown = context->mouseStatus.isRightDown;

	// If active units are of only one controller, we go to load map again, TODO win/loss screen
	int playerUnitsCount = 0;
	for (int i = 0; i < context->activeUnitCount; i++) {
		GameUnit *unit = context->activeUnits[i];
		if (!unit || unit->controller == UNIT_CONTROLLER_PLAYER) playerUnitsCount++;
	}
	if (playerUnitsCount == context->activeUnitCount || playerUnitsCount == 0) return GAME_STATE_LOAD_MAP;

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

	if (context->mouseStatus.isSelecting) {
		// Selection rectangle
		int selectionEndX = clamp(context->mouseStatus.x, VIEWPORT_X_MIN, VIEWPORT_X_MAX);
		int selectionEndY = clamp(context->mouseStatus.y, VIEWPORT_Y_MIN, VIEWPORT_Y_MAX);
		render_queue_submit_rect(renderQueue,
								 MOUSE_Z_ORDER,
								 selectionStartX, selectionStartY, selectionEndX, selectionEndY,
								 PAL_COLOR_GREEN);
	} else {
		// Mouse cursor
		render_queue_submit_sprite(renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(),
								   context->mouseStatus.x - mouse_x_focus, context->mouseStatus.y - mouse_y_focus, RND_FLAG_NORMAL);
	}

	snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
	render_queue_submit_text(renderQueue, UI_Z_ORDER + 510, font, fpsText, 0, 190, PAL_COLOR_WHITE, -1);

	return GAME_STATE_PLAY_MAP;
}

#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

int moveViewportCounter = 0;

typedef enum {
	SELECTION_SET,
	SELECTION_ADD,
	SELECTION_REMOVE
} SelectionModeEnum;

static void handle_viewport_mouse_action(GameContext *context, int mouseX, int mouseY, uint8_t isContextual) {
	// TODO spawn mouse confirmation object
	// Contextual action
	// Get board situation
	int boardXPosition = game_spatial_get_board_x_position(context->xPosition, mouseX);
	int boardYPosition = game_spatial_get_board_y_position(context->yPosition, mouseY);
	MouseCursorStateEnum mouseState = game_mouse_get_cursor_state();

	UnitId target = game_selection_get_in_position_or_previous(context, boardXPosition, boardYPosition);

	if (target < HANDLE_ID_THRESHOLD) {
		// Not unit position, depending on the tile, we move or interact with resource
		for (int i = 0; i < context->selectedUnitCount; i++) {
			GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
			if (!unit) continue;
			if (target == WALKABILITY_FREE) {
				if (isContextual) {
					game_unit_command_move(unit, NULL, boardXPosition, boardYPosition);
				} else {
					if (mouseState == MOUSE_CURSOR_ATTACK) game_unit_command_move_attack(unit, NULL, boardXPosition, boardYPosition);
					if (mouseState == MOUSE_CURSOR_TARGET) game_unit_command_move(unit, NULL, boardXPosition, boardYPosition);
				}
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
				if (isContextual) {
					if (targetUnit->controller == UNIT_CONTROLLER_AI) {
						game_unit_command_move_attack(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
					} else {
						game_unit_command_move(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
					}
				} else {
					if (mouseState == MOUSE_CURSOR_ATTACK) game_unit_command_move_attack(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
					if (mouseState == MOUSE_CURSOR_TARGET) game_unit_command_move(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
				}
			}
		}
	}
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
}

static void handle_viewport_select_all_of_same_type(GameContext *context, int mouseX, int mouseY) {
	// If we double click, select all units of the same type on screen
	int tileX = game_spatial_get_board_x_position(context->xPosition, mouseX);
	int tileY = game_spatial_get_board_y_position(context->yPosition, mouseY);
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
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
}

void handle_units_selection(GameContext *context, RenderQueue *renderQueue) {
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	MouseCursorStateEnum mouseCursor = game_mouse_get_cursor_state();
	
	// Actions that must be inside the area
	if (mouseY > VIEWPORT_Y_MIN && mouseY < VIEWPORT_Y_MAX &&
		mouseX > VIEWPORT_X_MIN && mouseX < VIEWPORT_X_MAX) {
		if (mouseCursor == MOUSE_CURSOR_IDLE || mouseCursor == MOUSE_CURSOR_LOOK || mouseCursor == MOUSE_CURSOR_SELECT) {
			if (context->mouseStatus.isLeftPressed) {
				game_mouse_start_selection(mouseX, mouseY);
				game_mouse_set_cursor_state(MOUSE_CURSOR_SELECT);
			}
			if (context->mouseStatus.isLeftDoubleClick) handle_viewport_select_all_of_same_type(context, mouseX, mouseY);
		}
		if (context->mouseStatus.isRightPressed & (mouseCursor == MOUSE_CURSOR_IDLE || mouseCursor == MOUSE_CURSOR_LOOK)) handle_viewport_mouse_action(context, mouseX, mouseY, TRUE);
		if (context->mouseStatus.isLeftPressed) {
			switch (mouseCursor) {
				case MOUSE_CURSOR_ATTACK:
				case MOUSE_CURSOR_TARGET:
					handle_viewport_mouse_action(context, mouseX, mouseY, FALSE);
					break;
				default:
					// Nothing else
					break;
			}
		}
	}

	if (!context->mouseStatus.isLeftDoubleClick && context->mouseStatus.isLeftReleased && mouseCursor == MOUSE_CURSOR_SELECT) {
		game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

		int selectionEndX = mouseX;
		int selectionEndY = mouseY;

		int dx = abs(selectionEndX - game_mouse_get_selection_start_x());
		int dy = abs(selectionEndY - game_mouse_get_selection_start_y());

		SelectionModeEnum selectionMode = SELECTION_SET;
		if (keyboard_is_key_down(KEY_LSHIFT) || keyboard_is_key_down(KEY_RSHIFT)) {
			selectionMode = SELECTION_ADD;
		} else {
			if (keyboard_is_key_down(KEY_LCONTROL) || keyboard_is_key_down(KEY_RCONTROL)) {
				selectionMode = SELECTION_REMOVE;
			}
		}

		if (dx < TILE_SIZE && dy < TILE_SIZE) {
			// Simple Click unitary
			int tileX = game_spatial_get_board_x_position(context->xPosition, game_mouse_get_selection_start_x());
			int tileY = game_spatial_get_board_y_position(context->yPosition, game_mouse_get_selection_start_y());

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
			int minScreenX = min_val(game_mouse_get_selection_start_x(), selectionEndX) - VIEWPORT_X_OFFSET;
			int maxScreenX = max_val(game_mouse_get_selection_start_x(), selectionEndX) - VIEWPORT_X_OFFSET;
			int minScreenY = min_val(game_mouse_get_selection_start_y(), selectionEndY) - VIEWPORT_Y_OFFSET;
			int maxScreenY = max_val(game_mouse_get_selection_start_y(), selectionEndY) - VIEWPORT_Y_OFFSET;

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

GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue) {
	// Inputs
	game_mouse_handle_status_change(context);

	// TODO menus
	if (keyboard_is_key_pressed(KEY_F12)) return GAME_STATE_EXIT;
	if (keyboard_is_key_pressed(KEY_F11)) return GAME_STATE_LOAD_MAP;
	if (keyboard_is_key_pressed(KEY_SPACE)) game_selection_center_camera_on_selection(context);
	// Resource debug keys
	if (keyboard_is_key_pressed(KEY_Y)) {
		resource_add_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 100);
	}
	if (keyboard_is_key_pressed(KEY_U)) {
		resource_add_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 100);
	}
	if (keyboard_is_key_pressed(KEY_J)) {
		resource_deduct_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 100);
	}
	if (keyboard_is_key_pressed(KEY_K)) {
		resource_deduct_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 100);
	}
	if (keyboard_is_key_pressed(KEY_L)) {
		context->config.lifeBar = (context->config.lifeBar + 1) % LIFE_BAR_COUNT;
	}

	// Selection slot handling
	game_selection_handle_slots(context);
	

	handle_units_selection(context, renderQueue);

	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;

	// If we click the mouseStatus on the minimap, we should move the camera or attack/move
	if (mouseX >= MINIMAP_X_POS &&
		mouseX <= MINIMAP_X_POS + BOARD_WIDTH &&
		mouseY >= MINIMAP_Y_POS &&
		mouseY <= MINIMAP_Y_POS + BOARD_HEIGHT) {
		MouseCursorStateEnum mouseState = game_mouse_get_cursor_state();

		if (context->mouseStatus.isLeftDown && mouseState == MOUSE_CURSOR_IDLE) {
			context->xPosition = (mouseX - MINIMAP_X_POS - 8) * TILE_SIZE;
			context->yPosition = (mouseY - MINIMAP_Y_POS - 6) * TILE_SIZE;
			if (context->yPosition < 0) context->yPosition = 0;
			if (context->xPosition < 0) context->xPosition = 0;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
		if ((mouseState == MOUSE_CURSOR_ATTACK || mouseState == MOUSE_CURSOR_TARGET) && context->mouseStatus.isLeftReleased) {
			uint16_t boardXPosition = clamp(mouseX - MINIMAP_X_POS, 0, BOARD_WIDTH);
			uint16_t boardYPosition = clamp(mouseY - MINIMAP_Y_POS, 0, BOARD_HEIGHT);
			for (int i = 0; i < context->selectedUnitCount; i++) {
				GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
				if (!unit) continue;
				if (mouseState == MOUSE_CURSOR_ATTACK) game_unit_command_move_attack(unit, NULL, boardXPosition, boardYPosition);
				if (mouseState == MOUSE_CURSOR_TARGET) game_unit_command_move(unit, NULL, boardXPosition, boardYPosition);
			}
			game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
		}
	}

	// Viewport moving
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

	game_unit_process_all(context);

	// Win condition
	// TODO WIN Message + WIN SCREEN
	// If active units are of only one controller, we go to load map again, TODO win/loss screen
	int playerUnitsCount = 0;
	for (int i = 0; i < context->activeUnitCount; i++) {
		GameUnit *unit = context->activeUnits[i];
		if (!unit || unit->controller == UNIT_CONTROLLER_PLAYER) playerUnitsCount++;
	}
	if (playerUnitsCount == context->activeUnitCount || playerUnitsCount == 0) return GAME_STATE_LOAD_MAP;


	

	// Logic
	game_objects_advance(context);
	game_strategy_ai_execute(context);
	resource_update_ui_quantities(context);

	// If there are more ticks to draw, skip queue phase, ups performance
	if (context->ticksToCatchup) return GAME_STATE_PLAY_MAP;

	// TODO should we have only one render function?
	render_queue_add_active_units(context, renderQueue);
	render_queue_add_active_objects(context, renderQueue);
	render_queue_submit_mouse(context, renderQueue);
	render_queue_submit_ui(context, renderQueue);

	return GAME_STATE_PLAY_MAP;
}

#include "selection.h"
typedef struct {
	UnitId selectedUnits[MAX_GAME_UNITS];
	uint16_t selectedUnitCount;
} SelectionSlot;
typedef enum {
	SELECTION_SLOT_1,
	SELECTION_SLOT_2,
	SELECTION_SLOT_3,
	SELECTION_SLOT_4,
	SELECTION_SLOT_5
} SelectionSlotIndexEnum;
#define MAX_SELECTION_SLOTS 5

SelectionSlot selectionSlots[MAX_SELECTION_SLOTS];

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

static void game_selection_remove_unit(GameContext *context, GameUnit *unit) {
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

static void game_selection_add_unit(GameContext *context, GameUnit *unit) {
	if (unit->isSelected) return;
	unit->isSelected = TRUE;
	context->selectedUnits[context->selectedUnitCount++] = unit->id;
}

static GameUnit *get_first_selected_unit_active(GameContext *context) {
	if (context->selectedUnitCount == 0) return NULL;
	for (int i = 0; i < context->selectedUnitCount; i++) {
		GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
		if (unit && unit->isActive) {
			return unit;
		}
	}
	return NULL;
}

static void game_selection_save_to_slot(GameContext *context, SelectionSlotIndexEnum slotIndex) {
	if (slotIndex >= MAX_SELECTION_SLOTS) return;

	SelectionSlot *slot = &selectionSlots[slotIndex];
	slot->selectedUnitCount = context->selectedUnitCount;
	for (int i = 0; i < context->selectedUnitCount; i++) {
		slot->selectedUnits[i] = context->selectedUnits[i];
	}
}

static void game_selection_load_from_slot(GameContext *context, SelectionSlotIndexEnum slotIndex) {
	if (slotIndex >= MAX_SELECTION_SLOTS) return;

	game_selection_clear(context);

	SelectionSlot *slot = &selectionSlots[slotIndex];
	for (int i = 0; i < slot->selectedUnitCount; i++) {
		GameUnit *unit = game_unit_get_by_id(context, slot->selectedUnits[i]);
		if (unit) {
			game_selection_add_unit(context, unit);
		}
	}

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
}

static void handle_target_selection(GameContext *context, GameUnit *unit, GameUnit *targetUnit) {
	if (unit->type == UNIT_TYPE_WORKER && targetUnit->isBuilding) {
		unit->typed.workerData.targetConstruction = targetUnit->id;
		if (game_spatial_unit_around_position(context, targetUnit->id, unit->x, unit->y)) {
			game_unit_command_work(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
			return;
		}
	}
	game_unit_command_move(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
}

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
					// TODO handle harvesting
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
				if (!unit || unit->isBuilding) continue;
				if (isContextual) {
					if (targetUnit->controller == UNIT_CONTROLLER_AI) {
						game_unit_command_move_attack(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
					} else {
						handle_target_selection(context, unit, targetUnit);
					}
				} else {
					if (mouseState == MOUSE_CURSOR_ATTACK) game_unit_command_move_attack(unit, targetUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
					if (mouseState == MOUSE_CURSOR_TARGET) handle_target_selection(context, unit, targetUnit);
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
	if (sourceUnit && !sourceUnit->isBuilding && sourceUnit->controller == UNIT_CONTROLLER_PLAYER) {
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

static void game_selection_clear_slots(GameContext *context) {
	for (int slot = 0; slot < MAX_SELECTION_SLOTS; slot++) {
		selectionSlots[slot].selectedUnitCount = 0;
	}
}

void game_selection_clear(GameContext *context) {
	for (int i = 0; i < context->selectedUnitCount; i++) {
		GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
		if (unit) unit->isSelected = FALSE;
	}
	context->selectedUnitCount = 0;
}

void game_selection_init(GameContext *context) {
	game_selection_clear(context);
	game_selection_clear_slots(context);
}

void game_selection_center_camera_on_selection(GameContext *context) {
	GameUnit *firstUnit = get_first_selected_unit_active(context);
	if (!firstUnit) return;

	// Center camera on the first selected unit
	int targetX = (firstUnit->x * TILE_SIZE) - (VIEWPORT_WIDTH / 2);
	int targetY = (firstUnit->y * TILE_SIZE) - (VIEWPORT_HEIGHT / 2);

	context->xPosition = clamp(targetX, 0, MAX_CAMERA_X_POSITION);
	context->yPosition = clamp(targetY, 0, MAX_CAMERA_Y_POSITION);
}

UnitId game_selection_get_in_position_or_previous(GameContext *context, int boardXPosition, int boardYPosition) {
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

void game_selection_handle_slots(GameContext *context) {
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
}

void game_selection_handle_input(GameContext *context) {
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
					if (foundUnit && !foundUnit->isBuilding && foundUnit->controller == UNIT_CONTROLLER_PLAYER) {
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
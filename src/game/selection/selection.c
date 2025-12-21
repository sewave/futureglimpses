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

static GameUnit* get_first_selected_unit_active(GameContext* context) {
    if(context->selectedUnitCount == 0) return NULL;
    for (int i = 0; i < context->selectedUnitCount; i++) {
        GameUnit* unit = game_unit_get_by_id(context, context->selectedUnits[i]);
        if (unit && unit->isActive) {
            return unit;
        }
    }
    return NULL;
}

static void game_selection_save_to_slot(GameContext* context, SelectionSlotIndexEnum slotIndex) {
    if (slotIndex >= MAX_SELECTION_SLOTS) return;

    SelectionSlot* slot = &selectionSlots[slotIndex];
    slot->selectedUnitCount = context->selectedUnitCount;
    for (int i = 0; i < context->selectedUnitCount; i++) {
        slot->selectedUnits[i] = context->selectedUnits[i];
    }
}

static void game_selection_clear_slots(GameContext *context) {
    for (int slot = 0; slot < MAX_SELECTION_SLOTS; slot++) {
        selectionSlots[slot].selectedUnitCount = 0;
    }
}

static void game_selection_load_from_slot(GameContext* context, SelectionSlotIndexEnum slotIndex) {
    if (slotIndex >= MAX_SELECTION_SLOTS) return;

    game_selection_clear(context);

    SelectionSlot* slot = &selectionSlots[slotIndex];
    for (int i = 0; i < slot->selectedUnitCount; i++) {
        GameUnit* unit = game_unit_get_by_id(context, slot->selectedUnits[i]);
        if (unit) {
            game_selection_add_unit(context, unit);
        }
    }
}

void game_selection_clear(GameContext *context) {
    for (int i = 0; i < context->selectedUnitCount; i++) {
        GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
        if (unit) unit->isSelected = FALSE;
    }
    context->selectedUnitCount = 0;
}

void game_selection_remove_unit(GameContext *context, GameUnit *unit) {
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

void game_selection_add_unit(GameContext *context, GameUnit *unit) {
	if (unit->isSelected) return;
	unit->isSelected = TRUE;
	context->selectedUnits[context->selectedUnitCount++] = unit->id;
}

void game_selection_center_camera_on_selection(GameContext* context) {
    GameUnit* firstUnit = get_first_selected_unit_active(context);
    if (!firstUnit) return;

    // Center camera on the first selected unit
    int targetX = (firstUnit->x * TILE_SIZE) - (VIEWPORT_WIDTH / 2);
    int targetY = (firstUnit->y * TILE_SIZE) - (VIEWPORT_HEIGHT / 2);

    context->xPosition = clamp(targetX, 0, MAX_CAMERA_X_POSITION);
    context->yPosition = clamp(targetY, 0, MAX_CAMERA_Y_POSITION);
}

UnitId game_selection_get_in_position_or_previous(GameContext* context, int boardXPosition, int boardYPosition) {
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

void game_selection_handle_slots(GameContext* context) {
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

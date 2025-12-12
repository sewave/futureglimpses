#include "selection.h"
typedef struct {
    UnitId selectedUnits[MAX_GAME_UNITS];
	uint16_t selectedUnitCount;
} SelectionSlot;

SelectionSlot selectionSlots[MAX_SELECTION_SLOTS];

void game_selection_clear(GameContext *context) {
    for (int i = 0; i < context->selectedUnitCount; i++) {
        GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
        if (unit) unit->isSelected = FALSE;
    }
    context->selectedUnitCount = 0;
}

void game_selection_clear_slots(GameContext *context) {
    for (int slot = 0; slot < MAX_SELECTION_SLOTS; slot++) {
        selectionSlots[slot].selectedUnitCount = 0;
    }
}

void game_selection_save_to_slot(GameContext* context, uint8_t slotIndex) {
    if (slotIndex >= MAX_SELECTION_SLOTS) return;

    SelectionSlot* slot = &selectionSlots[slotIndex];
    slot->selectedUnitCount = context->selectedUnitCount;
    for (int i = 0; i < context->selectedUnitCount; i++) {
        slot->selectedUnits[i] = context->selectedUnits[i];
    }
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

void game_selection_load_from_slot(GameContext* context, uint8_t slotIndex) {
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

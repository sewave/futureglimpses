#ifndef SELECTION_H
#define SELECTION_H
#include "../game_lib.h"
typedef enum {
    SELECTION_SLOT_1,
    SELECTION_SLOT_2,
    SELECTION_SLOT_3,
    SELECTION_SLOT_4,
    SELECTION_SLOT_5
} SelectionSlotIndexEnum;
#define MAX_SELECTION_SLOTS 5

void game_selection_clear(GameContext *context);
void game_selection_clear_slots(GameContext *context);
void game_selection_save_to_slot(GameContext* context, SelectionSlotIndexEnum slotIndex);
void game_selection_load_from_slot(GameContext* context, SelectionSlotIndexEnum slotIndex);
void game_selection_remove_unit(GameContext* context, GameUnit* unit);
void game_selection_add_unit(GameContext* context, GameUnit* unit);
void game_selection_center_camera_on_selection(GameContext* context);
GameUnit* game_selection_get_in_position_or_previous(GameContext* context, int boardXPosition, int boardYPosition);

#endif /* SELECTION_H */

#ifndef SELECTION_H
#define SELECTION_H
#include "../game_lib.h"
#define MAX_SELECTION_SLOTS 5

void game_selection_clear(GameContext *context);
void game_selection_clear_slots(GameContext *context);
void game_selection_save_to_slot(GameContext* context, uint8_t slotIndex);
void game_selection_load_from_slot(GameContext* context, uint8_t slotIndex);
void game_selection_remove_unit(GameContext* context, GameUnit* unit);
void game_selection_add_unit(GameContext* context, GameUnit* unit);

#endif /* SELECTION_H */

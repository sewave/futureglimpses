#ifndef SELECTION_H
#define SELECTION_H
#include "game/game_lib.h"

void game_selection_init(GameContext *context);
void game_selection_clear(GameContext *context);
void game_selection_add_unit(GameContext *context, GameUnit *unit);
void game_selection_center_camera_on_selection(GameContext* context);
UnitId game_selection_get_in_position_or_previous(GameContext* context, int boardXPosition, int boardYPosition);
void game_selection_handle_slots(GameContext* context);
void game_selection_handle_input(GameContext* context);
uint8_t game_selection_one_enemy_selected(GameContext* context);
uint8_t game_selection_one_own_building_selected(GameContext* context);

#endif /* SELECTION_H */

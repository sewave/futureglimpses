#ifndef SCENARIO_SELECT_H
#define SCENARIO_SELECT_H

#include "game/game.h"

void handle_scenario_select_init(GameContext *context);
GameStateEnum handle_scenario_select_update(GameContext *context);
void handle_scenario_select_render(GameContext *context, RenderQueue *renderQueue);

#endif /* SCENARIO_SELECT_H */

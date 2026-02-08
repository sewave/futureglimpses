#ifndef SCENARIO_SELECT_H
#define SCENARIO_SELECT_H

#include "game/game.h"

GameStateEnum handle_init_stage_select(GameContext *context, RenderQueue *renderQueue);
GameStateEnum handle_scenario_select(GameContext *context, RenderQueue *renderQueue);

#endif /* SCENARIO_SELECT_H */

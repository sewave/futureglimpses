#ifndef RESULTS_H
#define RESULTS_H
#include "game/game.h"

void handle_results_init(GameContext *context);
GameStateEnum handle_results_update(GameContext *context);
void handle_results_render(GameContext *context, RenderQueue *renderQueue);

#endif /* RESULTS_H */

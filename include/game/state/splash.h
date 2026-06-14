#ifndef SPLASH_H
#define SPLASH_H
#include "game/game.h"

void game_state_splash_init(GameContext *context);
GameStateEnum game_state_splash_update(GameContext *context);
void game_state_splash_render(GameContext *context, RenderQueue *renderQueue);

#endif /* SPLASH_H */

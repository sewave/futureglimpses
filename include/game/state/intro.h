#ifndef INTRO_H
#define INTRO_H
#include "game/game.h"

void game_state_intro_init(GameContext *context);
GameStateEnum game_state_intro_update(GameContext *context);
void game_state_intro_render(GameContext *context, RenderQueue *renderQueue);
void game_state_intro_exit(GameContext *context);

#endif /* INTRO_H */

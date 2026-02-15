#ifndef FADE_H
#define FADE_H
#include "game/game.h"

GameStateEnum handle_fade_in(GameContext *context, RenderQueue *renderQueue);
GameStateEnum handle_fade_out(GameContext *context, RenderQueue *renderQueue);
GameStateEnum fade_in_init(GameContext *context, GameStateEnum nextState, uint8_t speed);
GameStateEnum fade_out_init(GameContext *context, GameStateEnum nextState, uint8_t speed);

#endif /* FADE_H */

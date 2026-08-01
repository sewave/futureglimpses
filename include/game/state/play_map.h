#ifndef PLAY_MAP_H
#define PLAY_MAP_H
#include "game/game.h"

void handle_play_map_init(GameContext *context);
GameStateEnum handle_play_map_update(GameContext *context);
void handle_play_map_render(GameContext *context, RenderQueue *renderQueue);
void handle_play_map_exit(GameContext *context);

#endif /* PLAY_MAP_H */

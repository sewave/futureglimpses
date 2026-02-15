#ifndef TITLE_MAP_H
#define TITLE_MAP_H
#include "game/game.h"

void handle_title_init(GameContext *context);
GameStateEnum handle_title_update(GameContext *context);
void handle_title_render(GameContext *context, RenderQueue *renderQueue);

#endif /* TITLE_MAP_H */

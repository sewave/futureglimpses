#ifndef MENU_MAP_H
#define MENU_MAP_H
#include "game/game.h"

void handle_menu_map_init(GameContext *context);
GameStateEnum handle_menu_map_update(GameContext *context);
void handle_menu_map_render(GameContext *context, RenderQueue *renderQueue);

#endif /* MENU_MAP_H */

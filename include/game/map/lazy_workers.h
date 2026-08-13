#ifndef LAZY_WORKERS_H
#define LAZY_WORKERS_H
#include "game/game.h"

void lazy_workers_update(GameContext *context);
uint8_t lazy_workers_handle_input(GameContext *context);
void lazy_workers_render(GameContext *context, RenderQueue *renderQueue);

#endif /* LAZY_WORKERS_H */

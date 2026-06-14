#ifndef CMD_BAR_H
#define CMD_BAR_H
#include "game/game.h"

#define CMD_BAR_BUTTON_WIDTH 32
#define CMD_BAR_BUTTON_HEIGHT 22

void game_cmd_bar_handle_buttons(GameContext* context);
void game_cmd_bar_render_queue_submit(GameContext *context, RenderQueue* renderQueue);

#endif /* CMD_BAR_H */

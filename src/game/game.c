#include "game.h"

GameContext gameContext;

StateFunction gameStateTable[NUM_GAME_STATES] = {
    &handle_load_map,
    &handle_play_map,
    &handle_play_map
};

void game_free_game_state(GameContext *context) {
	destroy_bitmap(context->gameBack);
	destroy_bitmap(context->renderedBoard);
	destroy_bitmap(context->renderedMinimap);
	destroy_bitmap(context->tileSet);
}

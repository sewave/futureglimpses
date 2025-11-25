#include "game.h"

GameContext globalGameState;

StateFunction gameStateTable[NUM_GAME_STATES] = {
    &handle_load_map,
    &handle_play_map,
    &handle_play_map
};

void game_free_game_state(GameContext *gameState) {
	destroy_bitmap(gameState->gameBack);
    destroy_bitmap(gameState->renderedBoard);
    destroy_bitmap(gameState->renderedMinimap);
    destroy_bitmap(gameState->tileSet);
}

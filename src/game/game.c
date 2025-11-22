#include "game.h"

StateFunction gameStateTable[NUM_GAME_STATES] = {
    &handle_load_map,
    &handle_play_map,
    &handle_play_map
};

GameState globalGameState;

void game_free_game_state(GameState* gameState) {
    destroy_bitmap(gameState->gameBack);
    destroy_bitmap(gameState->renderedBoard);
    destroy_bitmap(gameState->renderedMinimap);
    destroy_bitmap(gameState->tileSet);
}

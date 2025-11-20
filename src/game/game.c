#include "game.h"

StateFunction gameStateTable[NUM_GAME_STATES] = {
    &handle_load_map,
    &handle_play_map,
    &handle_play_map
};

GameState globalGameState;

#include "game.h"

StateFunction gameStateTable[NUM_GAME_STATES] = {
    &handle_main_menu
};

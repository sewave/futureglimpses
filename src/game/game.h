#ifndef GAME_H
#define GAME_H

typedef enum {
    GAME_STATE_MAIN_MENU,
    NUM_GAME_STATES
} GameStateEnum;

typedef struct {
    GameStateEnum gameState;
} GameState;

typedef GameStateEnum (*StateFunction)(GameState*);

extern StateFunction gameStateTable[NUM_GAME_STATES];

extern GameStateEnum handle_main_menu(GameState* gameState);
#endif

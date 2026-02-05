#ifndef STAGE_SELECT_H
#define STAGE_SELECT_H

#include "game/game.h"

/**
 * Initializes the stage select state.
 * 
 * @param context Pointer to the game context
 * @param renderQueue Pointer to the render queue
 * @return The next game state
 */
GameStateEnum handle_init_stage_select(GameContext *context, RenderQueue *renderQueue);

/**
 * Handles the stage select state and processes user input.
 * 
 * @param context Pointer to the game context
 * @param renderQueue Pointer to the render queue
 * @return The next game state
 */
GameStateEnum handle_stage_select(GameContext *context, RenderQueue *renderQueue);

#endif /* STAGE_SELECT_H */

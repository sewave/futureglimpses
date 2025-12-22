#include "game.h"

//TODO: Do an initial load of all the assets (shouldn't be much)

extern GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue);
extern GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue);

StateFunction gameStateTable[NUM_GAME_STATES] = {
    &handle_load_map,
    &handle_play_map,
    &handle_play_map
};

void game_free_context(GameContext *context) {
	destroy_bitmap(context->gameBack);
	destroy_bitmap(context->renderedBoard);
	destroy_bitmap(context->renderedMinimap);
	destroy_bitmap(context->renderedMinimapUnits);
	destroy_bitmap(context->tileSet);
	destroy_bitmap(context->cmdButtons);
	destroy_font(context->gameFont);
}

GameStateEnum game_execute_state(GameContext *context, RenderQueue * renderQueue) {
	return gameStateTable[context->gameState](context, renderQueue);
}

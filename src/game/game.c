#include "game/game.h"

// TODO think how to remove extern
extern GameStateEnum handle_init_title(GameContext *context, RenderQueue *renderQueue);
extern GameStateEnum handle_title(GameContext *context, RenderQueue *renderQueue);
extern GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue);
extern GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue);
extern GameStateEnum handle_init_menu_map(GameContext *context, RenderQueue *renderQueue);
extern GameStateEnum handle_menu_map(GameContext *context, RenderQueue *renderQueue);

StateFunction gameStateTable[NUM_GAME_STATES] = {
	&handle_init_title,
	&handle_title,
    &handle_load_map,
    &handle_play_map,
	&handle_init_menu_map,
	&handle_menu_map,
    &handle_play_map
};

void game_free_context(GameContext *context) {
	destroy_bitmap(context->renderedBoard);
	destroy_bitmap(context->renderedMinimap);
	destroy_bitmap(context->renderedMinimapUnits);
	destroy_bitmap(context->screenBuffer);
	destroy_font(context->gameFont);
	if(context->map.title) free(context->map.title);
	if(context->map.description) free(context->map.description);
}

GameStateEnum game_execute_state(GameContext *context, RenderQueue * renderQueue) {
	return gameStateTable[context->gameState](context, renderQueue);
}

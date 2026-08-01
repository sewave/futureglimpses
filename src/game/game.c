#include "game/game.h"
#include "game/state/splash.h"
#include "game/state/intro.h"
#include "game/state/title.h"
#include "game/state/scenario_select.h"
#include "game/state/load_map.h"
#include "game/state/play_map.h"
#include "game/state/menu_map.h"
#include "game/state/results.h"

static GameState gameStatesTable[NUM_GAME_STATES] = {
		[GAME_STATE_SPLASH] = {.init = &game_state_splash_init, .update = &game_state_splash_update, .render = &game_state_splash_render, .exit = &game_state_splash_exit},
		[GAME_STATE_INTRO] = {.init = &game_state_intro_init, .update = &game_state_intro_update, .render = &game_state_intro_render, .exit = &game_state_intro_exit},
		[GAME_STATE_TITLE] = {.init = &handle_title_init, .update = &handle_title_update, .render = &handle_title_render, .exit = &handle_title_exit},
		[GAME_STATE_SCENARIO_SELECT] = {.init = &handle_scenario_select_init, .update = &handle_scenario_select_update, .render = &handle_scenario_select_render, .exit = &handle_scenario_select_exit},
		[GAME_STATE_LOAD_MAP] = {.init = NULL, .update = &handle_load_map_update, .render = NULL, .exit = NULL},
		[GAME_STATE_PLAY_MAP] = {.init = &handle_play_map_init, .update = &handle_play_map_update, .render = &handle_play_map_render, .exit = &handle_play_map_exit},
		[GAME_STATE_MENU_MAP] = {.init = &handle_menu_map_init, .update = &handle_menu_map_update, .render = &handle_menu_map_render, .exit = &handle_menu_map_exit},
		[GAME_STATE_RESULTS] = {.init = &handle_results_init, .update = &handle_results_update, .render = &handle_results_render, .exit = &handle_results_exit},
		[GAME_STATE_EXIT] = {.init = NULL, .update = NULL, .render = NULL, .exit = NULL} /* EXIT state */
};

void game_free_context(GameContext *context) {
	destroy_bitmap(context->renderedBoard);
	destroy_bitmap(context->renderedMinimap);
	destroy_bitmap(context->renderedMinimapUnits);
	destroy_bitmap(context->screenBuffer);
	destroy_font(context->gameFont);
	if (context->map.title) {
		free(context->map.title);
		context->map.title = NULL;
	}
	if (context->map.description) {
		free(context->map.description);
		context->map.description = NULL;
	}
}

GameStateEnum game_execute_state_update(GameContext *context) {
	StateUpdateFunction updateFunction = gameStatesTable[context->gameState].update;
	if (updateFunction) return updateFunction(context);
	return context->gameState;
}

void game_execute_state_init(GameContext *context) {
	StateInitFunction initFunction = gameStatesTable[context->gameState].init;
	if (initFunction) initFunction(context);
}

void game_execute_state_exit(GameContext *context) {
	StateExitFunction exitFunction = gameStatesTable[context->gameState].exit;
	if (exitFunction) exitFunction(context);
}

void game_execute_state_render(GameContext *context, RenderQueue *renderQueue) {
	StateRenderFunction renderFunction = gameStatesTable[context->gameState].render;
	if (renderFunction) renderFunction(context, renderQueue);
}

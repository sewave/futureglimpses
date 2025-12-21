#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

//TODO loading back, tileset and sprite sheets could be done on a previous init state and remain all game loaded
void spawn_test_units(GameContext *context) {
	context->xPosition = BOARD_WIDTH / 2 - VIEWPORT_WIDTH_TILES / 2;
	context->yPosition = BOARD_HEIGHT / 2 - VIEWPORT_HEIGHT_TILES / 2;

	for (int i = 0; i < MAX_GAME_UNITS / 2; i++) {
		while (!game_unit_spawn(context, i % 5, UNIT_CONTROLLER_AI,
								(uint16_t) random_int(BOARD_X_MIN, BOARD_X_MAX), (uint16_t) random_int(BOARD_Y_MIN, BOARD_Y_MAX))) {
			// Try until we find a free position
		}
		while (!game_unit_spawn(context, i % 5, UNIT_CONTROLLER_PLAYER,
								(uint16_t) random_int(BOARD_X_MIN, BOARD_X_MAX), (uint16_t) random_int(BOARD_Y_MIN, BOARD_Y_MAX))) {
			// Try until we find a free position
		}
	}
}

GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue) {

	if (context->gameBack) { destroy_bitmap(context->gameBack); }
	context->gameBack = load_bitmap("assets/ui/back_2.pcx", NULL);

	memset(context->boardExploration, BOARD_UNEXPLORED, sizeof(context->boardExploration));
	memset(context->walkabilityGrid, WALKABILITY_FREE, sizeof(context->walkabilityGrid));

	// TODO this file path should be in context, selected in a filebrowser
	MapData *map = game_map_load_data("assets/map/test.map");

	// Load the map here (for now we just fill it with random)
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			uint16_t tile = map->tile_layers->tiles[x + y * BOARD_WIDTH];
			context->board[x][y] = tile;
			if (tile > MAX_WALKABLE_TILE) context->walkabilityGrid[x][y] = WALKABILITY_BLOCKED;
			// TODO mark un resources table
		}
	}

	game_map_free_data(map);

	game_units_init(context);
	game_objects_init(context);
	game_selection_init(context);

	spawn_test_units(context);

	if (context->renderedBoard) { destroy_bitmap(context->renderedBoard); }
	context->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);

	if (context->renderedMinimap) { destroy_bitmap(context->renderedMinimap); }
	context->renderedMinimap = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	if (context->renderedMinimapUnits) { destroy_bitmap(context->renderedMinimapUnits); }
	context->renderedMinimapUnits = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	if (context->tileSet) { destroy_bitmap(context->tileSet); }
	context->tileSet = load_bitmap("assets/gfx/tileset.pcx", NULL);
	if (context->tileSet == NULL) {
		printf("Error loading tileset.");
		return GAME_STATE_EXIT;
	}
	// Generate the LUT for the tileset pixel colors
	BITMAP *minimapImage = load_bitmap("assets/gfx/tilesetm.pcx", NULL);
	if (context->tileSet == NULL) {
		printf("Error loading tileset minimap colors.");
		return GAME_STATE_EXIT;
	}
	int minimapColors[256];
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			minimapColors[x + y * 16] = getpixel(minimapImage, x, y);
		}
	}
	destroy_bitmap(minimapImage);

	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			int tile = context->board[x][y];
			blit(
					context->tileSet,
					context->renderedBoard,
					(tile % TILE_SIZE) * TILE_SIZE, (tile / TILE_SIZE) * TILE_SIZE,
					x * TILE_SIZE, y * TILE_SIZE,
					TILE_SIZE, TILE_SIZE);
			putpixel(context->renderedMinimap, x, y, minimapColors[tile]);
		}
	}

	resource_reset(context);
	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 500);
	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 300);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_GOLD, 500);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_WOOD, 300);

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

	return GAME_STATE_PLAY_MAP;
}

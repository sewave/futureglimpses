#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define MESSAGES_X 74
#define MESSAGES_Y 175
#define MESSAGES_Y_INC -14
#define MESSAGES_Z UI_Z_ORDER + 900

void spawn_test_units(GameContext *context) {
	context->xPosition = BOARD_WIDTH / 2 - VIEWPORT_WIDTH_TILES / 2;
	context->yPosition = BOARD_HEIGHT / 2 - VIEWPORT_HEIGHT_TILES / 2;
	GameUnit* unit = NULL;
	for (int i = 0; i < 1; i++) {
		do {
			unit = game_unit_spawn(context, i % 5, UNIT_CONTROLLER_AI,
								   (uint16_t) random_int(BOARD_X_MIN, BOARD_X_MAX), (uint16_t) random_int(BOARD_Y_MIN, BOARD_Y_MAX));
		} while (!unit);
		if (unit->isBuilding) {
			building_complete(context, unit);
			unit->health = unit->maxHealth;
		}
		unit = NULL;
		do {
			unit = game_unit_spawn(context, UNIT_TYPE_CITY_HALL + i % 6, UNIT_CONTROLLER_PLAYER,
								   (uint16_t) random_int(BOARD_X_MIN, BOARD_X_MAX), (uint16_t) random_int(BOARD_Y_MIN, BOARD_Y_MAX));
		} while (!unit);
		if (unit->isBuilding) {
			building_complete(context, unit);
			unit->health = unit->maxHealth;
		}
	}
}

GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue) {
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
			// TODO mark in resources table
		}
	}

	game_map_free_data(map);

	game_units_init(context);
	game_objects_init(context);
	game_selection_init(context);

	if (context->renderedBoard) { destroy_bitmap(context->renderedBoard); }
	context->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);

	if (context->renderedMinimap) { destroy_bitmap(context->renderedMinimap); }
	context->renderedMinimap = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	if (context->renderedMinimapUnits) { destroy_bitmap(context->renderedMinimapUnits); }
	context->renderedMinimapUnits = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	// Generate the LUT for the tileset pixel colors
	BITMAP *minimapImage = game_gfx_get_tileset_colors();
	int minimapColors[256];
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			minimapColors[x + y * 16] = getpixel(minimapImage, x, y);
		}
	}

	BITMAP* tileSet = game_gfx_get_tileset();
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			int tile = context->board[x][y];
			blit(
					tileSet,
					context->renderedBoard,
					(tile % TILE_SIZE) * TILE_SIZE, (tile / TILE_SIZE) * TILE_SIZE,
					x * TILE_SIZE, y * TILE_SIZE,
					TILE_SIZE, TILE_SIZE);
			putpixel(context->renderedMinimap, x, y, minimapColors[tile]);
		}
	}

	resource_reset(context);
	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 10000);
	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 10000);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_GOLD, 5000);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_WOOD, 3000);
	context->isDebugEnabled = FALSE;
	message_init(MESSAGES_X, MESSAGES_Y, MESSAGES_Y_INC, MESSAGES_Z);

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

	game_snd_play_music(GAME_MUSIC_MAP_1);

	spawn_test_units(context);

	return GAME_STATE_PLAY_MAP;
}

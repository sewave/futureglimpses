#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

//TODO loading back, tileset and sprite sheets could be done on a previous init state and remain all game loaded
static GameUnit archer = {

	};

GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue) {
	BITMAP* archerBitmap;



	// Spawn an idle archer
	// game_unit_spawn(context, &archer);


	context->gameBack = load_bitmap("assets/ui/back.pcx", NULL);

	// Reset the board exploration
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			context->boardExploration[x][y] = BOARD_UNEXPLORED;
		}
	}

	// TODO this file path should be in context, selected in a filebrowser
	MapData *map = game_map_load_data("assets/map/test.map");

	// Load the map here (for now we just fill it with random)
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			context->board[x][y] = map->tile_layers->tiles[x + y * BOARD_WIDTH];
		}
	}

	game_map_free_data(map);

	if (context->renderedBoard) { destroy_bitmap(context->renderedBoard); }
	context->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);

	if (context->renderedMinimap) { destroy_bitmap(context->renderedMinimap); }
	context->renderedMinimap = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

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
	return GAME_STATE_PLAY_MAP;
}

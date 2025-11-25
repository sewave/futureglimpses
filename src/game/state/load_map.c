#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

GameStateEnum handle_load_map(GameContext *gameState, RenderQueue *renderQueue) {
	gameState->gameBack = load_bitmap("assets/ui/back.pcx", NULL);

	// Reset the board exploration
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			gameState->boardExploration[x][y] = BOARD_UNEXPLORED;
		}
	}

	// TODO this should be in gameState, selected in a filebrowser
	MapData *map = game_map_load_data("assets/map/test.map");

	// Load the map here (for now we just fill it with random)
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			gameState->board[x][y] = map->tile_layers->tiles[x + y * BOARD_WIDTH];
		}
	}

	game_map_free_data(map);

	if (gameState->renderedBoard) { destroy_bitmap(gameState->renderedBoard); }
	gameState->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);

	if (gameState->renderedMinimap) { destroy_bitmap(gameState->renderedMinimap); }
	gameState->renderedMinimap = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	if (gameState->tileSet) { destroy_bitmap(gameState->tileSet); }
	gameState->tileSet = load_bitmap("assets/gfx/tileset.pcx", NULL);
	if (gameState->tileSet == NULL) {
		printf("Error loading tileset.");
		return GAME_STATE_EXIT;
	}
	// Generate the LUT for the tileset pixel colors
	BITMAP *minimapImage = load_bitmap("assets/gfx/tilesetm.pcx", NULL);
	if (gameState->tileSet == NULL) {
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
			int tile = gameState->board[x][y];
			blit(
					gameState->tileSet,
					gameState->renderedBoard,
					(tile % TILE_SIZE) * TILE_SIZE, (tile / TILE_SIZE) * TILE_SIZE,
					x * TILE_SIZE, y * TILE_SIZE,
					TILE_SIZE, TILE_SIZE);
			putpixel(gameState->renderedMinimap, x, y, minimapColors[tile]);
		}
	}
	return GAME_STATE_PLAY_MAP;
}

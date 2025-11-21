#include <stdio.h>
#include <stdlib.h>
#include "..\game_lib.h"
#include <allegro.h>

GameStateEnum handle_load_map(GameState *gameState, RenderQueue *renderQueue) {
	gameState->gameBack = load_bitmap("assets/ui/back.pcx", NULL);

	// Reset the board exploration
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			gameState->boardExploration[x][y] = BOARD_UNEXPLORED;
		}
	}

	// Load the map here (for now we just fill it with random)
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			gameState->board[x][y] = rand() % BOARD_UNKNOWN;
		}
	}

	if (gameState->renderedBoard) { destroy_bitmap(gameState->renderedBoard); }
	gameState->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);

	if (gameState->renderedMinimap) { destroy_bitmap(gameState->renderedMinimap); }
	gameState->renderedMinimap = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			int color = 0;
			switch (gameState->board[x][y]) {
				case BOARD_GRASS:
					color = makecol8(0, 255, 0);// Green for grass
					break;
				case BOARD_DIRT:
					color = makecol8(139, 69, 19);// Brown for dirt
					break;
				case BOARD_WATER:
					color = makecol8(0, 0, 255);// Blue for water
					break;
				case BOARD_MOUNTAIN:
					color = makecol8(128, 128, 128);// Gray for mountain
					break;
				case BOARD_ROAD:
					color = makecol8(210, 180, 140);// Tan for road
					break;
				case BOARD_FOREST:
					color = makecol8(34, 139, 34);// Forest green for forest
					break;
			}
			rectfill(gameState->renderedBoard,
					 x * TILE_SIZE, y * TILE_SIZE,
					 (x + 1) * TILE_SIZE - 1, (y + 1) * TILE_SIZE - 1,
					 color);
			putpixel(gameState->renderedMinimap, x, y, color);		 
		}
	}
	return GAME_STATE_PLAY_MAP;
}

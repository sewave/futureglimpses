#include <stdio.h>
#include <stdlib.h>
#include "..\game_lib.h"
#include <allegro.h>

GameStateEnum handle_load_map(GameState *gameState, RenderQueue *renderQueue) {

	// Reset the board exploration
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			gameState->boardExploration[x][y] = BOARD_UNEXPLORED;
		}
	}

	// Load the map here (for now we just fill it with zeros)
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			gameState->board[x][y] = rand() % BOARD_UNKNOWN;// Randomly assign tile types 0 or 1
		}
	}

	if (gameState->renderedBoard) { destroy_bitmap(gameState->renderedBoard); }
	gameState->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);

	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			int color = 0;
			switch (gameState->board[x][y]) {
				case BOARD_GRASS:
					color = makecol(0, 255, 0);// Green for grass
					break;
				case BOARD_DIRT:
					color = makecol(139, 69, 19);// Brown for dirt
					break;
				case BOARD_WATER:
					color = makecol(0, 0, 255);// Blue for water
					break;
				case BOARD_MOUNTAIN:
					color = makecol(128, 128, 128);// Gray for mountain
					break;
				case BOARD_ROAD:
					color = makecol(210, 180, 140);// Tan for road
					break;
				case BOARD_FOREST:
					color = makecol(34, 139, 34);// Forest green for forest
					break;
			}
			rectfill(gameState->renderedBoard,
					 x * TILE_SIZE,
					 y * TILE_SIZE,
					 (x + 1) * TILE_SIZE - 1,
					 (y + 1) * TILE_SIZE - 1,
					 color);
		}
	}
	return GAME_STATE_PLAY_MAP;
}

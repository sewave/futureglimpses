#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

//TODO loading back, tileset and sprite sheets could be done on a previous init state and remain all game loaded
static GameUnit tmpUnit = {
		.id = 0,
		.isActive = FALSE,
		.isBuilding = FALSE,
		.isSelected = FALSE,
		.type = UNIT_TYPE_SOLDIER,
		.controller = UNIT_CONTROLLER_PLAYER,
		.state = UNIT_STATE_IDLE,
		.nextState = UNIT_STATE_IDLE,
		.direction = DIRECTION_SOUTH,
		.x = 18,
		.y = 12,
		.attackRange = 1,
		.sightRange = 64,
		.health = 40,
		.maxHealth = 40,
		.minDamage = 3,
		.maxDamage = 5,
		.tileSize = 1,
		.targetX = NO_TARGET_POSITION,
		.targetY = NO_TARGET_POSITION,
		.targetId = NO_TARGET_ID,
		.reactionTime = 15,
		.reactionTimeCounter = 0,
		.moveTime = 35,
		.moveTimeCounter = 0,
		.animationStatus = {.sheet = NULL, .animation = {}, .frame = 0, .frameTicks = 0, .totalTicks = 0}};

void spawn_test_units(GameContext* context) {
	context->xPosition = 0 * TILE_SIZE;
	context->yPosition = 0 * TILE_SIZE;
	/*for(int i = 0; i < 128; i++) {
		game_unit_spawn(context, &tmpUnit);
		tmpUnit.controller = UNIT_CONTROLLER_AI;
		tmpUnit.x += 5;
		game_unit_spawn(context, &tmpUnit);
		tmpUnit.controller = UNIT_CONTROLLER_PLAYER;
		tmpUnit.x -= 5;
		tmpUnit.type = (tmpUnit.type + 1) % 5;
		tmpUnit.y++;
		if(tmpUnit.y > BOARD_HEIGHT) {
			tmpUnit.y = 0;
			tmpUnit.x += 15;
		}
	}*/

	for(int i = 0; i < 256; i++) {
		tmpUnit.controller = (tmpUnit.controller + 1) % 2;
		tmpUnit.type = (tmpUnit.type + 1) % 5;
		do {
			tmpUnit.x = (uint16_t) random_int(0, BOARD_WIDTH - 1);
			tmpUnit.y = (uint16_t) random_int(0, BOARD_WIDTH - 1);
		} while(context->walkabilityGrid[tmpUnit.x][tmpUnit.y] != WALKABILITY_FREE);
		game_unit_spawn(context, &tmpUnit);
	}
}

GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue) {
	
	context->gameBack = load_bitmap("assets/ui/back.pcx", NULL);

	memset(context->boardExploration, BOARD_UNEXPLORED, sizeof(context->boardExploration));
	memset(context->walkabilityGrid, WALKABILITY_FREE, sizeof(context->walkabilityGrid));

	// TODO this file path should be in context, selected in a filebrowser
	MapData *map = game_map_load_data("assets/map/test.map");

	// Load the map here (for now we just fill it with random)
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			uint16_t tile = map->tile_layers->tiles[x + y * BOARD_WIDTH];
			context->board[x][y] = tile;
			if(tile > MAX_WALKABLE_TILE) context->walkabilityGrid[x][y] = WALKABILITY_BLOCKED;
			// TODO marcar en tabla de recursos
		}
	}

	game_map_free_data(map);

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
	return GAME_STATE_PLAY_MAP;
}

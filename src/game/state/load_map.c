#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define MESSAGES_X 74
#define MESSAGES_Y 175
#define MESSAGES_Y_INC -14
#define MESSAGES_Z UI_Z_ORDER + 900

static const uint16_t quadrantXPositions[3] = { BOARD_WIDTH / 6, BOARD_WIDTH / 2, (BOARD_WIDTH * 5) / 6 };
static const uint16_t quadrantYPositions[3] = { BOARD_HEIGHT / 6, BOARD_HEIGHT / 2, (BOARD_HEIGHT * 5) / 6 };

static Position getQuadrantPosition(int xOff, int yOff) {
	uint16_t sourceX = quadrantXPositions[xOff];
    uint16_t sourceY = quadrantYPositions[yOff];

	uint16_t minX = sourceX - BOARD_WIDTH / 6;
	uint16_t minY = sourceY - BOARD_HEIGHT / 6;
    uint16_t maxX = sourceX + BOARD_WIDTH / 6;
	uint16_t maxY = sourceY + BOARD_HEIGHT / 6;
	return (Position) {.x = random_int(minX, maxX), .y= random_int(minY, maxY) };
}

void spawn_test_unit(GameContext *context, UnitTypeEnum type, ControllerEnum controller, int xOff, int yOff) {
	GameUnit *unit = NULL;
	do {
		Position pos = getQuadrantPosition(xOff, yOff);
		unit = game_unit_spawn(context, type, controller, pos.x, pos.y);
	} while (!unit);
	if (unit->isBuilding) {
		building_complete(context, unit);
		unit->health = unit->maxHealth;
	}
}

void spawn_test_units(GameContext *context) {
	context->xPosition = BOARD_WIDTH / 2 - VIEWPORT_WIDTH_TILES / 2;
	context->yPosition = BOARD_HEIGHT / 2 - VIEWPORT_HEIGHT_TILES / 2;

	int xOff = random_int(0, 2);
	int yOff = random_int(0, 2);

	for (int i = 0; i < 10; i++) {
		spawn_test_unit(context, i % 4 + UNIT_TYPE_SOLDIER, UNIT_CONTROLLER_AI, xOff, yOff);
	}

	int xOffPl, yOffPl;
	do {
		xOffPl = random_int(0, 2);
	    yOffPl = random_int(0, 2);
	} while(xOff == xOffPl && yOff == yOffPl);

	// Spawn 1 city hall and 5 workers for player
	spawn_test_unit(context, UNIT_TYPE_CITY_HALL, UNIT_CONTROLLER_PLAYER, xOffPl, yOffPl);
	for (int i = 0; i < 5; i++) {
		spawn_test_unit(context, UNIT_TYPE_WORKER, UNIT_CONTROLLER_PLAYER, xOffPl, yOffPl);
	}
}

static void load_map(GameContext *context, const char * filePath) {
	MapData *map = game_map_load_data(filePath);

	// Load the map here
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			uint16_t tile = map->tile_layers->tiles[x + y * BOARD_WIDTH];
			context->board[x][y] = tile;
			if (tile > MAX_WALKABLE_TILE) context->walkabilityGrid[x][y] = WALKABILITY_BLOCKED;
			// TODO mark in resources table
		}
	}

	// Only 1 object layer
	ObjectLayer *objLayer = &map->object_layers[0];
	for (int i = 0; i < objLayer->num_objects; i++) {
		MapObject *mapObj = &objLayer->objects[i];
		GameUnit *unit = game_unit_spawn(context, (UnitTypeEnum) mapObj->type, (ControllerEnum) mapObj->controller, mapObj->x, mapObj->y);
		if (unit->isBuilding) {
			building_complete(context, unit);
			unit->health = unit->maxHealth;
		}
	}

	//spawn_test_units(context);

	game_map_free_data(map);
}	

GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue) {
	memset(context->boardExploration, BOARD_UNEXPLORED, sizeof(context->boardExploration));
	memset(context->walkabilityGrid, WALKABILITY_FREE, sizeof(context->walkabilityGrid));

	game_units_init(context);
	game_objects_init(context);
	game_selection_init(context);

	load_map(context, "assets/map/test.map");

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
	context->gameResult = GAME_RESULT_ONGOING;
	message_init(MESSAGES_X, MESSAGES_Y, MESSAGES_Y_INC, MESSAGES_Z);

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

	game_snd_play_music(GAME_MUSIC_MAP_1);

	return GAME_STATE_PLAY_MAP;
}

#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include <allegro.h>

#define MESSAGES_X 74
#define MESSAGES_Y 175
#define MESSAGES_Y_INC -14
#define MESSAGES_Z UI_Z_ORDER + 900
#define MINIMAP_COLORS 256
#define TILESET_TILES_COLOR_WIDTH 16
#define TILESET_TILES_COLOR_HEIGHT 16

static void load_map(GameContext *context, const char * filePath) {
	MapData *map = game_map_load_data(filePath);

	// Load the map here
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			uint16_t tile = map->tileLayers->tiles[x + y * BOARD_WIDTH];
			context->board[x][y] = tile;
			if (tile > MAX_WALKABLE_TILE) context->walkabilityGrid[x][y] = WALKABILITY_BLOCKED;
			// TODO mark in resources table
		}
	}

	// Only 1 object layer
	ObjectLayer *objLayer = &map->objectLayers[0];
	for (int i = 0; i < objLayer->numObjects; i++) {
		MapObject *mapObj = &objLayer->objects[i];
		GameUnit *unit = game_unit_spawn(context, (UnitTypeEnum) mapObj->type, (ControllerEnum) mapObj->controller, mapObj->x, mapObj->y);
		if (unit && unit->isBuilding) {
			building_complete(context, unit);
			unit->health = unit->maxHealth;
		}
	}

	if(context->map.title) free(context->map.title);
	if(context->map.description) free(context->map.description);

	context->map.title = strdup(map->title);
	context->map.description = strdup(map->description);

	game_map_free_data(map);
}

static void render_minimap(GameContext *context) {
	// Generate the LUT for the tileset pixel colors
	BITMAP *minimapImage = game_gfx_get_tileset_colors();
	int minimapColors[MINIMAP_COLORS];
	for (int x = 0; x < TILESET_TILES_COLOR_WIDTH; x++) {
		for (int y = 0; y < TILESET_TILES_COLOR_HEIGHT; y++) {
			minimapColors[x + y * TILESET_TILES_COLOR_WIDTH] = getpixel(minimapImage, x, y);
		}
	}

	// TODO save lut to context for future minimap rendering

	BITMAP* tileSet = game_gfx_get_tileset();
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			int tile = context->board[x][y];
			blit(
					tileSet, context->renderedBoard,
					(tile % TILE_SIZE) * TILE_SIZE, (tile / TILE_SIZE) * TILE_SIZE,
					x * TILE_SIZE, y * TILE_SIZE,
					TILE_SIZE, TILE_SIZE);
			putpixel(context->renderedMinimap, x, y, minimapColors[tile]);
		}
	}
}

GameStateEnum handle_load_map(GameContext *context, RenderQueue *renderQueue) {
	memset(context->boardExploration, BOARD_UNEXPLORED, sizeof(context->boardExploration));
	memset(context->walkabilityGrid, WALKABILITY_FREE, sizeof(context->walkabilityGrid));

	game_units_init(context);
	game_objects_init(context);
	game_selection_init(context);
	resource_reset(context);

	// TODO load different maps based on campaign/scenario
	load_map(context, "assets/map/test.fgm");

	if (context->renderedBoard) { destroy_bitmap(context->renderedBoard); }
	context->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);

	if (context->renderedMinimap) { destroy_bitmap(context->renderedMinimap); }
	context->renderedMinimap = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	if (context->renderedMinimapUnits) { destroy_bitmap(context->renderedMinimapUnits); }
	context->renderedMinimapUnits = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	render_minimap(context);

	// TODO disable when resources can be harvested
	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 100000);
	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 100000);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_GOLD, 100000);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_WOOD, 100000);

	context->isDebugEnabled = FALSE;
	context->gameResult = GAME_RESULT_ONGOING;
	message_init(MESSAGES_X, MESSAGES_Y, MESSAGES_Y_INC, MESSAGES_Z);
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	game_snd_play_music(GAME_MUSIC_MAP_1);

	// Search first player active unit and move camera to it
	for (int i = 0; i < context->activeUnitCount; i++) {
		GameUnit *unit = context->activeUnits[i];
		if (unit && unit->controller == UNIT_CONTROLLER_PLAYER) {
			game_selection_add_unit(context, unit);
			game_selection_center_camera_on_selection(context);
			game_selection_clear(context);
			break;
		}
	}

	return GAME_STATE_PLAY_MAP;
}

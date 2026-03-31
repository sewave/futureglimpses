#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include <allegro.h>

#define MESSAGES_X 74
#define MESSAGES_Y 175
#define MESSAGES_Y_INC -14
#define MESSAGES_Z UI_Z_ORDER + 900

typedef struct {
	uint16_t minTile, maxTile;
	int16_t altTileOffset;
	TileTypeEnum tileType;
	uint16_t data;
} BoardTileDefinition;

static BoardTileDefinition boardTileConversion[] = {
		{.minTile = 0x00, .maxTile = 0x7F, .altTileOffset = 0, .tileType = TILE_TYPE_WALKABLE, .data = 0},
		{.minTile = 0x80, .maxTile = 0x8F, .altTileOffset = -0x20, .tileType = TILE_TYPE_WOOD, .data = 100},
		{.minTile = 0x90, .maxTile = 0x9F, .altTileOffset = -0x20, .tileType = TILE_TYPE_GOLD, .data = 2000},
		{.minTile = 0xA0, .maxTile = 0xBF, .altTileOffset = 0, .tileType = TILE_TYPE_BLOCKED, .data = 0},
		{.minTile = 0xC0, .maxTile = 0xDF, .altTileOffset = -0xC0, .tileType = TILE_TYPE_WALL, .data = 50},
};

#define BOARD_TILE_CONVERSION_ELEMENTS sizeof(boardTileConversion) / sizeof(boardTileConversion[0])

static BoardTile get_board_tile(uint16_t tile) {
	BoardTileDefinition definition = boardTileConversion[0];

	BoardTileDefinition* definitionPtr = &boardTileConversion[0];
	for(int i = 0; i < BOARD_TILE_CONVERSION_ELEMENTS; i++, definitionPtr++) {
		if(tile >= definitionPtr->minTile && tile <= definitionPtr->maxTile) {
			definition = *definitionPtr;
			break;
		}
	}

	return (BoardTile) {
		.tile = tile,
		.data = definition.data,
		.type = definition.tileType,
		.altTile = tile + definition.altTileOffset
	};
}

static InitializationStatusEnum load_map(GameContext *context, const char * filePath) {
	MapData *map = game_map_load_data(filePath);
	if(!map) return INITIALIZATION_ERROR;

	// Load the map here
	for (int x = 0; x < BOARD_WIDTH; x++) {
		for (int y = 0; y < BOARD_HEIGHT; y++) {
			uint16_t tile = map->tileLayers->tiles[x + y * BOARD_WIDTH];
			BoardTile boardTile = get_board_tile(tile);
			context->board[x][y] = boardTile;
			if (boardTile.type == TILE_TYPE_WALKABLE) {
				context->walkabilityGrid[x][y] = WALKABILITY_FREE;
			}
			else {
				context->walkabilityGrid[x][y] = WALKABILITY_BLOCKED;
			}
		}
	}

	// Only one object layer
	ObjectLayer *objLayer = &map->objectLayers[0];
	for (int i = 0; i < objLayer->numObjects; i++) {
		MapObject *mapObj = &objLayer->objects[i];
		GameUnit *unit = game_unit_spawn(context, (UnitTypeEnum) mapObj->type, (ControllerEnum) mapObj->controller, mapObj->x, mapObj->y);
		if (unit) {
			if(unit->isBuilding) {
				building_complete(context, unit);
				unit->health = unit->maxHealth;
			}
			if(mapObj->isCustom) {
				unit->isCustom = TRUE;
				strcpy(unit->name, mapObj->name);
				if(mapObj->maxHealth) {
					unit->maxHealth = clamp(mapObj->maxHealth, UNIT_MIN_HEALTH, UNIT_MAX_HEALTH);
					unit->health = unit->maxHealth;
				}
				if(mapObj->minDamage) unit->minDamage = clamp(mapObj->minDamage, UNIT_MIN_DAMAGE, UNIT_MAX_DAMAGE);
				if(mapObj->maxDamage) unit->maxDamage = clamp(mapObj->maxDamage, UNIT_MIN_DAMAGE, UNIT_MAX_DAMAGE);
				unit->mustSurvive = mapObj->mustSurvive;
			}
		}
	}

	free(context->map.title);
	free(context->map.description);
	free(context->map.win);
	free(context->map.lose);
	context->map.title = strdup(map->title);
	context->map.description = strdup(map->description);
	context->map.win = strdup(map->win);
	context->map.lose = strdup(map->lose);

	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, map->playerGold);
	resource_set_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, map->playerWood);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_GOLD, map->computerGold);
	resource_set_amount(context, UNIT_CONTROLLER_AI, RESOURCE_TYPE_WOOD, map->computerWood);

	context->map.enableBarracks = map->enableBarracks;
	context->map.enableBlacksmith = map->enableBlacksmith;
	context->map.enableFarm = map->enableFarm;
	context->map.enableStables = map->enableStables;
	context->map.enableTower = map->enableTower;
	context->map.aiMode = (AIModeEnum) map->aiMode;
	// Only half of the frames check attack
	context->map.peaceTime = SEC_TO_FRAMES(map->peaceTime) / 2;

	game_map_free_data(map);
	context->targetBlinkTime = 0;
	return INITIALIZATION_OK;
}

GameStateEnum handle_load_map_update(GameContext *context) {
	memset(context->boardExploration, BOARD_UNEXPLORED, sizeof(context->boardExploration));
	memset(context->walkabilityGrid, WALKABILITY_FREE, sizeof(context->walkabilityGrid));

	game_units_init(context);
	game_objects_init(context);
	game_selection_init(context);
	resource_reset(context);

	if (context->renderedBoard) { destroy_bitmap(context->renderedBoard); }
	context->renderedBoard = create_bitmap(BOARD_WIDTH * TILE_SIZE, BOARD_HEIGHT * TILE_SIZE);
	rectfill(context->renderedBoard, 0, 0, context->renderedBoard->w, context->renderedBoard->h, PAL_COLOR_BLACK);

	if (context->renderedMinimap) { destroy_bitmap(context->renderedMinimap); }
	context->renderedMinimap = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);
	rectfill(context->renderedMinimap, 0, 0, context->renderedMinimap->w, context->renderedMinimap->h, PAL_COLOR_BLACK);

	if (context->renderedMinimapUnits) { destroy_bitmap(context->renderedMinimapUnits); }
	context->renderedMinimapUnits = create_bitmap(BOARD_WIDTH, BOARD_HEIGHT);

	if(load_map(context, context->mapPath) == INITIALIZATION_ERROR) {
		TRACE("Error loading map: %s\n", context->mapPath);
		return GAME_STATE_SCENARIO_SELECT;
	}

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

	for (int i = 0; i < context->activeUnitCount; i++) {
		GameUnit *unit = context->activeUnits[i];
		if (unit && unit->controller == UNIT_CONTROLLER_PLAYER) game_unit_explore(context, unit);
	}

	return GAME_STATE_PLAY_MAP;
}

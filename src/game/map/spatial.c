#include "game/map/spatial.h"
#include "common/common.h"
#include "game/unit/unit.h"
#include "game/video/render.h"
#include "game/video/gfx.h"
#include "game/video/game_video.h"

static int game_spatial_get_exploration_sorroundings(GameContext *context, int x, int y) {
	int xMin = x - 1;
	int xMax = x + 1;
	int yMin = y - 1;
	int yMax = y + 1;

	int surroundings = BOARD_PARTIALLY_EXPLORED;
	if (xMin < BOARD_X_MIN || (xMin >= BOARD_X_MIN && xMin <= BOARD_X_MAX && context->boardExploration[xMin][y] == BOARD_UNEXPLORED)) {
		surroundings |= PARTIAL_EXPLORE_LEFT_MASK;
	}
	if (xMax > BOARD_X_MAX || (xMax >= BOARD_X_MIN && xMax <= BOARD_X_MAX && context->boardExploration[xMax][y] == BOARD_UNEXPLORED)) {
		surroundings |= PARTIAL_EXPLORE_RIGHT_MASK;
	}
	if (yMin < BOARD_Y_MIN || (yMin >= BOARD_Y_MIN && yMin <= BOARD_Y_MAX && context->boardExploration[x][yMin] == BOARD_UNEXPLORED)) {
		surroundings |= PARTIAL_EXPLORE_UP_MASK;
	}
	if (yMax > BOARD_Y_MAX || (yMax >= BOARD_Y_MIN && yMax <= BOARD_Y_MAX && context->boardExploration[x][yMax] == BOARD_UNEXPLORED)) {
		surroundings |= PARTIAL_EXPLORE_DOWN_MASK;
	}

	return surroundings;
}

uint16_t game_spatial_query_grid(GameContext* context, uint16_t centerTileX, uint16_t centerTileY, uint8_t tileRadius,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 GameUnit* foundUnits[], uint16_t maxResults) {
	int tileMinX = clamp(centerTileX - tileRadius, BOARD_X_MIN, BOARD_X_MAX);
	int tileMaxX = clamp(centerTileX + tileRadius, BOARD_X_MIN, BOARD_X_MAX);
	int tileMinY = clamp(centerTileY - tileRadius, BOARD_Y_MIN, BOARD_Y_MAX);
	int tileMaxY = clamp(centerTileY + tileRadius, BOARD_Y_MIN, BOARD_Y_MAX);

	return game_spatial_query_grid_rectangle(context, 
								tileMinX, tileMinY,
								tileMaxX, tileMaxY,
								 filterFunc, sourceUnit,
								 foundUnits, maxResults);
}

uint16_t game_spatial_query_grid_rectangle(GameContext* context, 
								uint16_t tileMinX, uint16_t tileMinY,
								uint16_t tileMaxX, uint16_t tileMaxY,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 GameUnit* foundUnits[], uint16_t maxResults) {
    uint16_t resultCount = 0;

	unsigned char alreadyAdded[MAX_GAME_UNITS];
	memset(alreadyAdded, FALSE, sizeof(alreadyAdded));

	for (int col = tileMinX; col <= tileMaxX; col++) {
		for (int row = tileMinY; row <= tileMaxY; row++) {
			UnitId id = context->walkabilityGrid[col][row];

			// Only units
			if (id < HANDLE_ID_THRESHOLD) continue;

			GameUnit *foundUnit = game_unit_get_by_id(context, id);

			if (foundUnit && foundUnit != sourceUnit) {
				int entityIndex = GET_INDEX(id);

				if (alreadyAdded[entityIndex]) continue;

				if (filterFunc == NULL || filterFunc(context, sourceUnit, foundUnit)) {
					if (resultCount < maxResults) {
						foundUnits[resultCount++] = foundUnit;
						alreadyAdded[entityIndex] = TRUE;
					}
				}
			}
		}
	}

	return resultCount;
}

uint8_t game_spatial_filter_enemy_units(const GameContext* context, const GameUnit *source, const GameUnit *found) {
	return source->isActive && found->isActive && source->controller != found->controller && found->state != UNIT_STATE_DIE;
}

uint8_t game_spatial_filter_enemy_units_in_attack_range(const GameContext* context, const GameUnit *source, const GameUnit *found) {
	return source->isActive && found->isActive && source->controller != found->controller
	&& found->state != UNIT_STATE_DIE && game_spatial_unit_target_in_attack_range(source, found);
}

uint8_t game_spatial_filter_construction_sites(const GameContext* context, const GameUnit *source, const GameUnit *found) {
	return found->isActive && found->controller == source->controller && found->isBuilding
		&& found->state == BUILDING_STATE_CONSTRUCT;
}

uint8_t game_spatial_unit_in_range(const GameUnit * source, const GameUnit* target, uint8_t range) {
	if(range == 0) return FALSE;
	// If the range is 1, we do a bounding box check, otherwise we do a distance check
	if(range == 1) {
		int sourceMinX = clamp(source->x - 1, BOARD_X_MIN, BOARD_X_MAX);
		int sourceMaxX = clamp(source->x + source->tileSize, BOARD_X_MIN, BOARD_X_MAX);
		int sourceMinY = clamp(source->y - 1, BOARD_Y_MIN, BOARD_Y_MAX);
		int sourceMaxY = clamp(source->y + source->tileSize, BOARD_Y_MIN, BOARD_Y_MAX);
		int targetMinX = clamp(target->x, BOARD_X_MIN, BOARD_X_MAX);
		int targetMaxX = clamp(target->x + target->tileSize - 1, BOARD_X_MIN, BOARD_X_MAX);
		int targetMinY = clamp(target->y, BOARD_Y_MIN, BOARD_Y_MAX);
		int targetMaxY = clamp(target->y + target->tileSize - 1, BOARD_Y_MIN, BOARD_Y_MAX);
		// Return true if the bounding boxes overlap
		return !(sourceMaxX < targetMinX || sourceMinX > targetMaxX
			|| sourceMaxY < targetMinY || sourceMinY > targetMaxY);
	}
	else {
		int sourceTileHalfSize = (source->tileSize - 1) / 2;
		int targetTileHalfSize = (target->tileSize - 1) / 2;
		int sourceCenterX = source->x + sourceTileHalfSize;
		int sourceCenterY = source->y + sourceTileHalfSize;
		int targetCenterX = target->x + targetTileHalfSize;
		int targetCenterY = target->y + targetTileHalfSize;
		int dx = abs(sourceCenterX - targetCenterX);
		int dy = abs(sourceCenterY - targetCenterY);
		int expandedRange = range + sourceTileHalfSize + targetTileHalfSize;
		return expandedRange * expandedRange >= dx * dx + dy * dy;
	}
}

uint8_t game_spatial_target_in_range(const GameUnit * source, uint16_t targetX, uint16_t targetY, uint8_t range) {
	return abs(((int) source->x) - targetX) <= range && abs(((int) source->y) - targetY) <= range;
}

uint16_t game_spatial_get_board_x_position(uint16_t cameraPosition, int cursorPosition) {
	return clamp((cameraPosition + cursorPosition - VIEWPORT_X_OFFSET) / TILE_SIZE, BOARD_X_MIN, BOARD_X_MAX);
}

uint16_t game_spatial_get_board_y_position(uint16_t cameraPosition, int cursorPosition) {
	return clamp((cameraPosition + cursorPosition - VIEWPORT_Y_OFFSET) / TILE_SIZE, BOARD_Y_MIN, BOARD_Y_MAX);
}

#define POSITION_SORROUNDINGS 8

static Position sorroundings[POSITION_SORROUNDINGS] = {
	{-1, -1}, {0, -1}, {1, -1},
	{-1, 0}, {1, 0}, 
	{-1, 1}, {0, 1}, {1, 1},
};

uint8_t game_spatial_unit_around_position(GameContext* context, UnitId unitId, uint16_t x, uint16_t y) {
	for(int i = 0; i < POSITION_SORROUNDINGS; i++) {
		Position offsets = sorroundings[i];
		int checkX = (int) x + offsets.x;
		int checkY = (int) y + offsets.y;
		if (checkX < BOARD_X_MIN || checkX > BOARD_X_MAX || checkY < BOARD_Y_MIN || checkY > BOARD_Y_MAX) continue;
		if (context->walkabilityGrid[checkX][checkY] == unitId) return TRUE;
	}
	return FALSE;
}

uint8_t game_spatial_unit_in_explored_viewport(const GameContext *context, const GameUnit *unit) {
	return context->boardExploration[unit->x][unit->y] == BOARD_EXPLORED &&
		unit->x >= context->xPosition / TILE_SIZE && unit->x <= (context->xPosition + VIEWPORT_WIDTH) / TILE_SIZE &&
		unit->y >= context->yPosition / TILE_SIZE && unit->y <= (context->yPosition + VIEWPORT_HEIGHT) / TILE_SIZE;
}

uint8_t game_spatial_object_in_explored_viewport(const GameContext *context, const Object *object) {
	return context->boardExploration[object->x / TILE_SIZE][object->y / TILE_SIZE] == BOARD_EXPLORED &&
		   object->x >= context->xPosition && object->x <= (context->xPosition + VIEWPORT_WIDTH) &&
		   object->y >= context->yPosition && object->y <= (context->yPosition + VIEWPORT_HEIGHT);
}

uint8_t game_spatial_unit_target_in_attack_range(const GameUnit* unit, const GameUnit* target) {
	return game_spatial_unit_in_range(unit, target, unit->maxAttackRange) &&
		!game_spatial_unit_in_range(unit, target, unit->minAttackRange);
}

uint8_t game_spatial_target_in_attack_range(GameUnit* unit, int targetX, int targetY) {
	return (game_spatial_target_in_range(unit, targetX, targetY, unit->maxAttackRange) &&
		!game_spatial_target_in_range(unit, targetX, targetY, unit->minAttackRange));
}

void game_spatial_explore_position(GameContext* context, uint16_t x, uint16_t y) {
	BoardTile *boardTile = &context->board[x][y];
	uint16_t tile = boardTile->tile;
	blit(game_gfx_get_tileset(), context->renderedBoard,
			(tile % TILE_SIZE) * TILE_SIZE, tile & TILE_SIZE_16_MASK,
			x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
	putpixel(context->renderedMinimap, x, y, context->minimapColors[tile]);
	context->boardExploration[x][y] = BOARD_EXPLORED;
}

void game_spatial_explore_partial_position(GameContext* context, uint16_t x, uint16_t y, int partialExploration) {
	BoardTile *boardTile = &context->board[x][y];
	uint16_t tile = boardTile->tile;
	blit(game_gfx_get_tileset(), context->renderedBoard,
			(tile % TILE_SIZE) * TILE_SIZE, tile & TILE_SIZE_16_MASK,
			x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
	context->boardExploration[x][y] = partialExploration;
	int tileX = x * TILE_SIZE;
	int tileY = y * TILE_SIZE;
	if(partialExploration & PARTIAL_EXPLORE_UP_MASK) {
		masked_blit(game_gfx_get_overtile(GAME_OVERTILE_UP), context->renderedBoard,
			0, 0, tileX, tileY, TILE_SIZE, TILE_SIZE);
	}
	if (partialExploration & PARTIAL_EXPLORE_DOWN_MASK) {
		masked_blit(game_gfx_get_overtile(GAME_OVERTILE_DOWN), context->renderedBoard,
			0, 0, tileX, tileY, TILE_SIZE, TILE_SIZE);
	}
	if (partialExploration & PARTIAL_EXPLORE_LEFT_MASK) {
		masked_blit(game_gfx_get_overtile(GAME_OVERTILE_LEFT), context->renderedBoard,
			0, 0, tileX, tileY, TILE_SIZE, TILE_SIZE);
	}
	if (partialExploration & PARTIAL_EXPLORE_RIGHT_MASK) {
		masked_blit(game_gfx_get_overtile(GAME_OVERTILE_RIGHT), context->renderedBoard,
			0, 0, tileX, tileY, TILE_SIZE, TILE_SIZE);
	}
	putpixel(context->renderedMinimap, x, y, PAL_COLOR_DARK_GRAY);
}

void game_spatial_explore_radius(GameContext *context, int xCenter, int yCenter, int range) {
	int xMin = clamp(xCenter - range, BOARD_X_MIN, BOARD_X_MAX);
	int xMax = clamp(xCenter + range, BOARD_X_MIN, BOARD_X_MAX);
	int yMin = clamp(yCenter - range, BOARD_Y_MIN, BOARD_Y_MAX);
	int yMax = clamp(yCenter + range, BOARD_Y_MIN, BOARD_Y_MAX);
	for (int x = xMin; x <= xMax; x++) {
		int *boardExplorationRow = &context->boardExploration[x][yMin];
		for (int y = yMin; y <= yMax; y++, boardExplorationRow++) {
			if (*boardExplorationRow != BOARD_EXPLORED) {
				int dx = abs(x - xCenter);
				int dy = abs(y - yCenter);
				if (dx != range || dy != range) {
					if (dx == range || dy == range) {
						if (*boardExplorationRow == BOARD_UNEXPLORED) *boardExplorationRow = NO_DIRECTION_PARTIAL_EXPLORE;
					} else {
						game_spatial_explore_position(context, x, y);
					}
				}
			}
		}
	}
	xMin = clamp(xMin - 1, BOARD_X_MIN, BOARD_X_MAX);
	xMax = clamp(xMax + 1, BOARD_X_MIN, BOARD_X_MAX);
	yMin = clamp(yMin - 1, BOARD_Y_MIN, BOARD_Y_MAX);
	yMax = clamp(yMax + 1, BOARD_Y_MIN, BOARD_Y_MAX);
	for (int x = xMin; x <= xMax; x++) {
		int *boardExplorationRow = &context->boardExploration[x][yMin];
		for (int y = yMin; y <= yMax; y++, boardExplorationRow++) {
			int boardExplorationValue = *boardExplorationRow;
			if (boardExplorationValue >= BOARD_PARTIALLY_EXPLORED) {
				int newSurroundings = game_spatial_get_exploration_sorroundings(context, x, y);
				if(newSurroundings == BOARD_PARTIALLY_EXPLORED) {
					game_spatial_explore_position(context, x, y);
				} else {
					if (newSurroundings != boardExplorationValue) {
						game_spatial_explore_partial_position(context, x, y, newSurroundings);
					}
				}
			}
		}
	}
}

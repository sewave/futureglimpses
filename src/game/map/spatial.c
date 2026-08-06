#include "game/map/spatial.h"
#include "common/common.h"
#include "game/unit/unit.h"
#include "game/video/render.h"
#include "game/video/gfx.h"

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
	BoardTile *tile = &context->board[x][y];
	blit(game_gfx_get_tileset(), context->renderedBoard,
			(tile->tile % TILE_SIZE) * TILE_SIZE, (tile->tile / TILE_SIZE) * TILE_SIZE,
			x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
	putpixel(context->renderedMinimap, x, y, context->minimapColors[tile->tile]);
	context->boardExploration[x][y] = BOARD_EXPLORED;
}

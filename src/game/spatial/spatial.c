#include "spatial.h"

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

			if (foundUnit != sourceUnit) {
				int entityIndex = GET_INDEX(id);

				if (alreadyAdded[entityIndex]) continue;

				if (col == foundUnit->x && row == foundUnit->y) {
					if (filterFunc == NULL || filterFunc(context, sourceUnit, foundUnit)) {
						if (resultCount < maxResults) {
							foundUnits[resultCount++] = foundUnit;
							alreadyAdded[entityIndex] = TRUE;
						}
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

uint8_t game_spatial_unit_in_range(const GameUnit * source, const GameUnit* target, uint8_t range) {
	uint8_t sourceTileRadius = source->tileSize / 2;
	uint8_t targetTileRadius = target->tileSize / 2;
	uint32_t xDist = (source->x + sourceTileRadius) - (target->x + targetTileRadius);
	uint32_t yDist = (source->y + sourceTileRadius) - (target->y + targetTileRadius);
	uint32_t expandedRange = range + sourceTileRadius + targetTileRadius;
	return expandedRange * expandedRange >= (xDist * xDist + yDist * yDist);
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

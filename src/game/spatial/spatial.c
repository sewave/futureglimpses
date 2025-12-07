#include "spatial.h"

uint16_t game_spatial_query_grid(GameContext* context, uint16_t centerTileX, uint16_t centerTileY, uint8_t tileRadius,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 UnitId foundIds[], uint16_t maxResults) {
    uint16_t resultCount = 0;

	// 1. Calculate the tile bounding box
	int tileMinX = clamp(centerTileX - tileRadius, 0, BOARD_WIDTH - 1);
	int tileMaxX = clamp(centerTileX + tileRadius, 0, BOARD_WIDTH - 1);
	int tileMinY = clamp(centerTileY - tileRadius, 0, BOARD_HEIGHT - 1);
	int tileMaxY = clamp(centerTileY + tileRadius, 0, BOARD_HEIGHT - 1);

	// Array para evitar duplicados, ya que varias tiles pueden apuntar al mismo id
	unsigned char alreadyAdded[MAX_GAME_UNITS];
	memset(alreadyAdded, FALSE, sizeof(alreadyAdded));

	// 2. Iterate through the bounding box of tiles
	for (int row = tileMinY; row <= tileMaxY; row++) {
		for (int col = tileMinX; col <= tileMaxX; col++) {
			int id = context->walkabilityGrid[col][row];

			// Only units
			if (id < HANDLE_ID_THRESHOLD) continue;

			GameUnit *foundUnit = game_unit_get_by_id(context, id);

			if (foundUnit) {
				int entityIndex = GET_INDEX(id);

				if (alreadyAdded[entityIndex]) continue;

				// 3. Apply custom filter function only on anchor tile
				if (col == foundUnit->x && row == foundUnit->y) {
					if (filterFunc == NULL || filterFunc(NULL, sourceUnit, foundUnit)) {
						if (resultCount < maxResults) {
							foundIds[resultCount++] = foundUnit->id;
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
	uint32_t xDist = source->x - targetX;
	uint32_t yDist = source->y - targetY;
	return range * range >= (xDist * xDist + yDist * yDist);
}


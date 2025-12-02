#include "spatial.h"

uint16_t game_spatial_query_grid(const GameContext* context, uint16_t centerTileX, uint16_t centerTileY, uint8_t tileRadius,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 UnitId outHandles[], uint16_t maxResults) {
    // TODO
	return 0;
}

uint8_t game_spatial_filter_enemy_units(const GameContext* context, const GameUnit *source, const GameUnit *found) {
	return source->isActive && found->isActive && source->controller != found->controller;
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


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
	uint32_t xDist = source->x - target->x;
	uint32_t yDist = source->y - target->y;
	return ((uint32_t) range) * ((uint32_t) range) < (xDist * xDist + yDist * yDist);
}

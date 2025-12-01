#include "spatial.h"

uint16_t game_spatial_query_grid(const GameContext* context, uint16_t centerTileX, uint16_t centerTileY, uint8_t tileRadius,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 UnitId outHandles[], uint16_t maxResults) {
	return 0;
}

uint8_t game_spatial_filter_enemy_units(const GameContext* context, const GameUnit *source, const GameUnit *found) {
	return source->isActive && found->isActive && source->controller != found->controller;
}

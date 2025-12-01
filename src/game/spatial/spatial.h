#ifndef SPATIAL_H
#define SPATIAL_H
#include "../game_lib.h"

/**
 * All filter functions must conform to this 3-argument signature.
 * The context is the first parameter.
 * Type: Contextual Filter.
 */
typedef uint8_t (*GenericQueryFilterFunc)(const GameContext *context, const GameUnit *candidate, const GameUnit *source);

uint16_t game_spatial_query_grid(const GameContext* context, uint16_t centerTileX, uint16_t centerTileY, uint8_t tileRadius,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 UnitId outHandles[], uint16_t maxResults);

uint8_t game_spatial_filter_enemy_units(const GameContext* context, const GameUnit *source, const GameUnit *found);

#endif /* SPATIAL_H */

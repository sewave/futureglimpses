#ifndef SPATIAL_H
#define SPATIAL_H
#include "../game_lib.h"

/**
 * All filter functions must conform to this 3-argument signature.
 * The context is the first parameter.
 * Type: Contextual Filter.
 */
typedef uint8_t (*GenericQueryFilterFunc)(const GameContext *context, const GameUnit *candidate, const GameUnit *source);

uint16_t game_spatial_query_grid(GameContext* context, uint16_t centerTileX, uint16_t centerTileY, uint8_t tileRadius,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 GameUnit* foundUnits[], uint16_t maxResults);

uint16_t game_spatial_query_grid_rectangle(GameContext* context, 
								uint16_t tileMinX, uint16_t tileMinY,
								uint16_t tileMaxX, uint16_t tileMaxY,
								 GenericQueryFilterFunc filterFunc, const GameUnit *sourceUnit,
								 GameUnit* foundUnits[], uint16_t maxResults);

uint8_t game_spatial_filter_enemy_units(const GameContext* context, const GameUnit *source, const GameUnit *found);
uint8_t game_spatial_filter_enemy_units_in_attack_range(const GameContext* context, const GameUnit *source, const GameUnit *found);

uint8_t game_spatial_unit_in_range(const GameUnit * source, const GameUnit* target, uint8_t range);

uint8_t game_spatial_target_in_range(const GameUnit * source, uint16_t targetX, uint16_t targetY, uint8_t range);

uint16_t game_spatial_get_board_x_position(uint16_t cameraPosition, int cursorPosition);

uint16_t game_spatial_get_board_y_position(uint16_t cameraPosition, int cursorPosition);

#endif /* SPATIAL_H */

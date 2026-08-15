#ifndef SPATIAL_H
#define SPATIAL_H
#include "game/game.h"

#define NO_DIRECTION_PARTIAL_EXPLORE BOARD_PARTIALLY_EXPLORED + 0x80
#define PARTIAL_EXPLORE_UP_MASK 0x40
#define PARTIAL_EXPLORE_DOWN_MASK 0x20
#define PARTIAL_EXPLORE_LEFT_MASK 0x10
#define PARTIAL_EXPLORE_RIGHT_MASK 0x08

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
uint8_t game_spatial_filter_construction_sites(const GameContext* context, const GameUnit *source, const GameUnit *found);
uint8_t game_spatial_unit_in_range(const GameUnit * source, const GameUnit* target, uint8_t range);
uint8_t game_spatial_target_in_range(const GameUnit * source, uint16_t targetX, uint16_t targetY, uint8_t range);
uint16_t game_spatial_get_board_x_position(uint16_t cameraPosition, int cursorPosition);
uint16_t game_spatial_get_board_y_position(uint16_t cameraPosition, int cursorPosition);
uint8_t game_spatial_unit_around_position(GameContext* context, UnitId unitId, uint16_t x, uint16_t y);
uint8_t game_spatial_unit_in_explored_viewport(const GameContext *context, const GameUnit *unit);
uint8_t game_spatial_object_in_explored_viewport(const GameContext *context, const Object *object);
uint8_t game_spatial_unit_target_in_attack_range(const GameUnit* unit, const GameUnit* target);
uint8_t game_spatial_target_in_attack_range(GameUnit* unit, int targetX, int targetY);
void game_spatial_explore_position(GameContext* context, uint16_t x, uint16_t y);
void game_spatial_explore_partial_position(GameContext* context, uint16_t x, uint16_t y, int partialExploration);
void game_spatial_explore_radius(GameContext *context, int xCenter, int yCenter, int range);

#endif /* SPATIAL_H */

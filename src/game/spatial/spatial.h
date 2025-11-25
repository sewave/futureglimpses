#ifndef SPATIAL_H
#define SPATIAL_H
#include "../game_lib.h"

/**
 * All filter functions must conform to this 3-argument signature.
 * The context is the first parameter.
 * Type: Contextual Filter.
 */
typedef unsigned char (*GenericQueryFilterFunc)(const GameContext *context, const GameUnit *candidate, const GameUnit *sourceUnit);

void spatial_clear_grid(GameContext *context);
int spatial_get_units_in_radius(float centerX, float centerY, float radius, int* results, int maxResults, 
                        GenericQueryFilterFunc filterFunc, int sourceHandle, GameContext* context);
int spatial_get_units_in_area(float x1In, float y1In, float x2In, float y2In, int* results, int maxResults, 
                      GenericQueryFilterFunc filterFunc, int sourceHandle, GameContext* context);

#endif /* SPATIAL_H */

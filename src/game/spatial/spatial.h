#ifndef SPATIAL_H
#define SPATIAL_H
#include "../game_lib.h"

/**
 * All filter functions must conform to this 3-argument signature.
 * The context is the first parameter.
 * Type: Contextual Filter.
 */
typedef unsigned char (*GenericQueryFilterFunc)(const GameContext *context, const GameUnit *candidate, const GameUnit *source);


#endif /* SPATIAL_H */

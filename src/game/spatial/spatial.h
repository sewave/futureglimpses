#ifndef SPATIAL_H
#define SPATIAL_H
#include "../game_lib.h"

void spatial_clear_grid(GameState* gameState);
int spatial_get_nearby_units(GameState* gameState, GameUnit* me, int* results, int maxResults);

#endif /* SPATIAL_H */

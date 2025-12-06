#include "unit_path.h"

#define NO_DIRECTION -1
#define UNIT_DIRS 8
#define UNIT_DIR_AXIS 2
static const uint8_t unit_movements[UNIT_DIRS][UNIT_DIR_AXIS] = {
		{0, 1},
		{1, 1},
		{1, 0},
		{1, -1},
		{0, -1},
		{-1, -1},
		{-1, 0},
		{-1, 1}};

static int game_unit_path_get_best_direction(GameUnit *unit, int targetX, int targetY) {
	int best_dir = NO_DIRECTION;
	int min_dist_sq = 999999999;
	for (int i = 0; i < UNIT_DIRS; i++) {
		int nextX = unit->x + unit_movements[i][0];
		int nextY = unit->y + unit_movements[i][1];
		int dist = distance_sq(nextX, nextY, targetX, targetY);

		if (dist < min_dist_sq) {
			min_dist_sq = dist;
			best_dir = i;
		}
	}
	return best_dir;
}

uint8_t game_unit_path_find(GameContext* context, GameUnit* unit, uint16_t targetX, uint16_t targetY) {
    int8_t dir = game_unit_path_get_best_direction(unit, targetX, targetY);
    if(dir == NO_DIRECTION) return FALSE;

    unit->prevX = unit->x;
    unit->prevY = unit->y;

    // Clear previous position on grid
    context->walkabilityGrid[unit->prevY][unit->prevX] = WALKABILITY_FREE;

    // Instant movement, animation will be slower
    unit->x = unit_movements[dir][0];
    unit->y = unit_movements[dir][1];

    // Mark new position as occupied
    context->walkabilityGrid[unit->y][unit->x] = unit->id;
    unit->direction = dir;

    return TRUE;
}

#include "unit_path.h"

#define DIAGONAL_MULTIPLIER 14
#define DIAGONAL_DIVIDER 10
#define DIAGONAL_DIR_MOD 2

#define NO_DIRECTION -1
#define UNIT_DIRS 8
#define UNIT_DIR_VARS 3
#define UNIT_DIR_X 0
#define UNIT_DIR_Y 1
#define UNIT_DIR_DIR 2
static const int8_t unitMovements[UNIT_DIRS][UNIT_DIR_VARS] = {
		{0, -1,  DIRECTION_NORTH},
		{1, -1,  DIRECTION_NORTH},
		{1,  0,  DIRECTION_EAST},
		{1,  1,  DIRECTION_SOUTH},
		{0,  1,  DIRECTION_SOUTH},
		{-1, 1,  DIRECTION_SOUTH},
		{-1, 0,  DIRECTION_WEST},
		{-1, -1, DIRECTION_NORTH}
};

#define NO_PARENT -1
#define MAX_NODES BOARD_WIDTH * BOARD_HEIGHT
#define MAX_PATH 64
#define OCCUPIED_TILE_MIN 128

static int8_t game_unit_path_get_best_direction(GameContext *context, GameUnit *unit, int targetX, int targetY) {
	int8_t bestDir = NO_DIRECTION;
	int minDistSq = 999999999;
	for (int8_t i = 0; i < UNIT_DIRS; i++) {
		int nextX = unit->x + unitMovements[i][UNIT_DIR_X];
		int nextY = unit->y + unitMovements[i][UNIT_DIR_Y];
        if(nextX < 0 || nextY < 0 || nextX >= BOARD_WIDTH || nextY >= BOARD_HEIGHT) continue;
		int dist = distance_sq(nextX, nextY, targetX, targetY);

		if (context->walkabilityGrid[nextX][nextY] == WALKABILITY_FREE && dist < minDistSq) {
			minDistSq = dist;
			bestDir = i;
		}
	}
	return bestDir;
}

uint8_t game_unit_path_find(GameContext *context, GameUnit *unit, uint16_t targetX, uint16_t targetY) {
	int8_t dir = game_unit_path_get_best_direction(context, unit, targetX, targetY);
	if (dir == NO_DIRECTION) return FALSE;
	context->walkabilityGrid[unit->x][unit->y] = WALKABILITY_FREE;
	unit->prevX = unit->x;
	unit->prevY = unit->y;      
	unit->x += unitMovements[dir][UNIT_DIR_X];
	unit->y += unitMovements[dir][UNIT_DIR_Y];
	context->walkabilityGrid[unit->x][unit->y] = unit->id;
	unit->direction = unitMovements[dir][UNIT_DIR_DIR];
	if(dir % DIAGONAL_DIR_MOD) {
		// Diagonal movement, slower
		unit->moveTimeAnim = (unit->moveTime * DIAGONAL_MULTIPLIER) / DIAGONAL_DIVIDER;
	}
	else {
		unit->moveTimeAnim = unit->moveTime;
	}
	return TRUE;
}

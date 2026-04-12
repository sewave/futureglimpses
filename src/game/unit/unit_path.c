#include "game/unit/unit_path.h"

#define DIAGONAL_MULTIPLIER 45
#define DIAGONAL_DIVIDER 32
#define DIAGONALS_INDEX 5
#define DIRECTION_NONE 0
#define PROXIMITY_SEARCH_MIN -1
#define PROXIMITY_SEARCH_MAX 1

/* Directions: 0:None, 1:N, 2:S, 3:E, 4:W, 5:NE, 6:NW, 7:SE, 8:SW */
static const int8_t directionDx[] = {0, 0, 0, 1, -1, 1, -1, 1, -1};
static const int8_t directionDy[] = {0, -1, 1, 0, 0, -1, -1, 1, 1};
static const int8_t directionFacing[] = {0, DIRECTION_NORTH, DIRECTION_SOUTH, DIRECTION_EAST, DIRECTION_WEST, DIRECTION_NORTH, DIRECTION_NORTH, DIRECTION_SOUTH, DIRECTION_SOUTH};

static int game_unit_path_evaluate_direction(GameContext *context, GameUnit *unit, uint16_t nextX, uint16_t nextY, int8_t moveDir) {
	int distDx = nextX - unit->pathfindingData.pathTargetX;
	int distDy = nextY - unit->pathfindingData.pathTargetY;
	int distSq = (distDx * distDx) + (distDy * distDy);
	int score = distSq * 10;

	uint16_t *xPos = unit->pathfindingData.lastPositionsX;
	uint16_t *yPos = unit->pathfindingData.lastPositionsY;
	for (int i = 0; i < PATHFINDING_HISTORY_SIZE; i++, xPos++, yPos++) {
		if (*xPos == NO_TARGET_POSITION || *yPos == NO_TARGET_POSITION) continue;
		if (nextX == *xPos && nextY == *yPos) {
			score += 999;
			break;
		}
	}

	if (moveDir == unit->pathfindingData.lastDir) score -= 30;
	if (moveDir >= DIAGONALS_INDEX) score += 25;
	if (unit->pathfindingData.frustration > 10) score += (rand() % (unit->pathfindingData.frustration * 16));

	return score;
}

void game_unit_path_init(GameUnit *unit, uint16_t targetX, uint16_t targetY) {
	unit->pathfindingData.lastDir = 0;
	unit->pathfindingData.historyIdx = 0;
	unit->pathfindingData.frustration = 0;
	unit->pathfindingData.pathTargetX = targetX;
	unit->pathfindingData.pathTargetY = targetY;

	for (int i = 0; i < PATHFINDING_HISTORY_SIZE; i++) {
		unit->pathfindingData.lastPositionsX[i] = NO_TARGET_POSITION;
		unit->pathfindingData.lastPositionsY[i] = NO_TARGET_POSITION;
	}

	int dx = unit->x - targetX;
	int dy = unit->y - targetY;
	unit->pathfindingData.lastDistSq = (dx * dx) + (dy * dy);
}

static int8_t game_unit_path_get_best_direction(GameContext *context, GameUnit *unit, int targetX, int targetY) {
	int bestDir = DIRECTION_NONE;
	int bestScore = 2000000000;

	unit->pathfindingData.lastPositionsX[unit->pathfindingData.historyIdx] = unit->x;
	unit->pathfindingData.lastPositionsY[unit->pathfindingData.historyIdx] = unit->y;
	unit->pathfindingData.historyIdx = (unit->pathfindingData.historyIdx + 1) % PATHFINDING_HISTORY_SIZE;

	for (int i = DIRECTION_NONE + 1; i <= 8; i++) {
		int nextX = unit->x + directionDx[i];
		int nextY = unit->y + directionDy[i];

		if (nextX < 0 || nextX >= BOARD_WIDTH || nextY < 0 || nextY >= BOARD_HEIGHT || context->walkabilityGrid[nextX][nextY] == WALKABILITY_BLOCKED) continue;

		int currentScore = game_unit_path_evaluate_direction(context, unit, nextX, nextY, i);
		if (currentScore < bestScore) {
			bestScore = currentScore;
			bestDir = i;
		}
	}

	if (bestDir != DIRECTION_NONE) {
		int nextX = unit->x + directionDx[bestDir];
		int nextY = unit->y + directionDy[bestDir];

		if (context->walkabilityGrid[nextX][nextY] == WALKABILITY_FREE) {
			unit->pathfindingData.lastDir = bestDir;
			int pathDistX = nextX - unit->pathfindingData.pathTargetX;
			int pathDistY = nextY - unit->pathfindingData.pathTargetY;
			int currentDistSq = (pathDistX * pathDistX) + (pathDistY * pathDistY);

			if (currentDistSq >= unit->pathfindingData.lastDistSq) {
				unit->pathfindingData.frustration++;
			} else {
				if (unit->pathfindingData.frustration > 0) unit->pathfindingData.frustration--;
			}
			unit->pathfindingData.lastDistSq = currentDistSq;
			return bestDir;
		} else {
			unit->pathfindingData.frustration += 2;
		}
	} else {
		unit->pathfindingData.frustration += 5;
	}
	return DIRECTION_NONE;
}

uint8_t game_unit_path_find(GameContext *context, GameUnit *unit, uint16_t targetX, uint16_t targetY) {
	if (unit->pathfindingData.pathTargetX != targetX || unit->pathfindingData.pathTargetY != targetY) {
		game_unit_path_init(unit, targetX, targetY);
	}

	int8_t dir = game_unit_path_get_best_direction(context, unit, targetX, targetY);
	if (dir == DIRECTION_NONE) return FALSE;
	context->walkabilityGrid[unit->x][unit->y] = WALKABILITY_FREE;
	unit->prevX = unit->x;
	unit->prevY = unit->y;
	unit->x += directionDx[dir];
	unit->y += directionDy[dir];
	context->walkabilityGrid[unit->x][unit->y] = unit->id;
	unit->direction = directionFacing[dir];
	if (dir >= DIAGONALS_INDEX) {
		// Diagonal movement, slower
		unit->moveTimeAnim = (unit->moveTime * DIAGONAL_MULTIPLIER) / DIAGONAL_DIVIDER;
	} else {
		unit->moveTimeAnim = unit->moveTime;
	}
	return TRUE;
}

#include "spatial.h"

void spatial_clear_grid(GameState *gameState) {
	memset(gameState->buckets, UNUSED_BUCKET_ID, sizeof(gameState->buckets));
}

void spatial_register_unit(GameState *gameState, GameUnit *u) {
	// Calculate the bounding box edges in world coordinates (AABB)
	float xMin = u->x - u->width / 2.0f;
	float yMin = u->y - u->height / 2.0f;
	float xMax = u->x + u->width / 2.0f;
	float yMax = u->y + u->height / 2.0f;

	// Clamp world edges
	if (xMin < 0) xMin = 0;
	if (yMin < 0) yMin = 0;
	if (xMax > WORLD_WIDTH) xMax = WORLD_WIDTH;
	if (yMax > WORLD_HEIGHT) yMax = WORLD_HEIGHT;

	// Determine the range of grid cells the unit overlaps
	int startCol = (int) (xMin * INVERSE_TILE_SIZE);
	int endCol = (int) (xMax * INVERSE_TILE_SIZE);
	int startRow = (int) (yMin * INVERSE_TILE_SIZE);
	int endRow = (int) (yMax * INVERSE_TILE_SIZE);

	// Clamp grid indices
	if (startCol < 0) startCol = 0;
	if (startRow < 0) startRow = 0;
	if (endCol >= BOARD_WIDTH) endCol = BOARD_WIDTH - 1;
	if (endRow >= BOARD_HEIGHT) endRow = BOARD_HEIGHT - 1;

	int index = GET_INDEX(u->id);
	// Iterate and register the unit in ALL overlapping cells
	for (int r = startRow; r <= endRow; r++) {
		int cellIndex = r * BOARD_WIDTH + startCol;
		for (int c = startCol; c <= endCol; c++) {
			gameState->nextUnitIndices[index] = gameState->buckets[cellIndex];
			gameState->buckets[cellIndex] = index;
			++cellIndex;
		}
	}
}

int spatial_get_nearby_units(GameState *gameState, GameUnit *me, int *results, int maxResults) {
	int count = 0;
	int myCol = (int) (me->x * INVERSE_TILE_SIZE);
	int myRow = (int) (me->y * INVERSE_TILE_SIZE);
	int myIndex = GET_INDEX(me->id);

	for (int yOffset = -1; yOffset <= 1; yOffset++) {
		for (int xOffset = -1; xOffset <= 1; xOffset++) {
			int targetCol = myCol + xOffset;
			int targetRow = myRow + yOffset;

			if (targetCol < 0 || targetCol >= BOARD_WIDTH || targetRow < 0 || targetRow >= BOARD_HEIGHT) continue;

			int cellIndex = targetRow * BOARD_WIDTH + targetCol;
			int currentIndex = gameState->buckets[cellIndex];

			while (currentIndex != -1) {
				if (currentIndex != myIndex) {
					// Valid unit check is implicit because we rebuild grid every frame,
					// but we can double check active state if needed.
					if (count < maxResults) {
						// We return the full handle, not just the index, so external code is safe
						results[count++] = gameState->units[currentIndex].id;
					}
				}
				currentIndex = gameState->nextUnitIndices[currentIndex];
			}
		}
	}
	return count;
}

int spatial_get_units_in_area(GameState *gameState, float x1In, float y1In, float x2In, float y2In, int *results, int maxResults) {
	int count = 0;

	// --- 1. SANITY CHECK / NORMALIZE INPUT COORDINATES ---
	float x1 = (x1In < x2In) ? x1In : x2In;
	float x2 = (x1In < x2In) ? x2In : x1In;
	float y1 = (y1In < y2In) ? y1In : y2In;
	float y2 = (y1In < y2In) ? y2In : y1In;

	// 2. Determine Grid Bounds of the Query Box (Camera View)
	int startCol = (int) (x1 * INVERSE_TILE_SIZE);
	int endCol = (int) (x2 * INVERSE_TILE_SIZE);
	int startRow = (int) (y1 * INVERSE_TILE_SIZE);
	int endRow = (int) (y2 * INVERSE_TILE_SIZE);

	// 3. Clamp the bounds to the World Grid (important for edges)
	if (startCol < 0) startCol = 0;
	if (startRow < 0) startRow = 0;
	if (endCol >= BOARD_WIDTH) endCol = BOARD_WIDTH - 1;
	if (endRow >= BOARD_HEIGHT) endRow = BOARD_HEIGHT - 1;

	unsigned char alreadyProcessed[MAX_GAME_UNITS] = {FALSE};

	// 4. Iterate ONLY the relevant buckets
	for (int r = startRow; r <= endRow; r++) {
		for (int c = startCol; c <= endCol; c++) {
			int cellIndex = r * BOARD_WIDTH + c;
			int unitIdx = gameState->buckets[cellIndex];

			// Walk the linked list for this cell
			while (unitIdx != -1) {
				GameUnit *u = &gameState->units[unitIdx];
				int unitHandleIndex = GET_INDEX(u->id);

				// Check 1: Has this unit already been added by an adjacent bucket?
				if (alreadyProcessed[unitHandleIndex]) {
					unitIdx = gameState->nextUnitIndices[unitIdx];
					continue;
				}

				// Check 2: Fine-grained center-point visibility check
				if (is_point_in_box(u->x, u->y, x1, y1, x2, y2)) {
					if (count < maxResults) {
						results[count++] = u->id;
						alreadyProcessed[unitHandleIndex] = TRUE;
					} else {
						return count;
					}
				}
				unitIdx = gameState->nextUnitIndices[unitIdx];
			}
		}
	}
	return count;
}

int get_units_in_radius(GameState *gameState, float centerX, float centerY, float radius, int *results, int maxResults, int excludeHandle) {
	int count = 0;

	// 1. Calculate the squared radius (to avoid slow sqrtf calls)
	float radiusSq = radius * radius;

	// 2. Determine the Bounding Box of the circle
	float x1 = centerX - radius;
	float y1 = centerY - radius;
	float x2 = centerX + radius;
	float y2 = centerY + radius;

	// 3. Clamp World Edges
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 > WORLD_WIDTH) x2 = WORLD_WIDTH;
	if (y2 > WORLD_HEIGHT) y2 = WORLD_HEIGHT;

	// 4. Determine Grid Bounds (Optimized with inverse multiplication)
	int startCol = (int) (x1 * INVERSE_TILE_SIZE);
	int endCol = (int) (x2 * INVERSE_TILE_SIZE);
	int startRow = (int) (y1 * INVERSE_TILE_SIZE);
	int endRow = (int) (y2 * INVERSE_TILE_SIZE);

	// 5. Clamp Grid Indices
	if (startCol < 0) startCol = 0;
	if (startRow < 0) startRow = 0;
	if (endCol >= BOARD_WIDTH) endCol = BOARD_WIDTH - 1;
	if (endRow >= BOARD_HEIGHT) endRow = BOARD_HEIGHT - 1;

	unsigned char alreadyProcessed[MAX_GAME_UNITS] = {FALSE};

	// 6. Iterate ONLY the relevant buckets
	for (int r = startRow; r <= endRow; r++) {
		for (int c = startCol; c <= endCol; c++) {

			int cellIndex = r * BOARD_WIDTH + c;
			int unitIdx = gameState->buckets[cellIndex];

			// Walk the linked list for this cell
			while (unitIdx != -1) {
				GameUnit *u = &gameState->units[unitIdx];
				int unitHandleIndex = GET_INDEX(u->id);

				// Check 1: Already processed or excluding self
				if (alreadyProcessed[unitHandleIndex] || u->id == excludeHandle) {
					unitIdx = gameState->nextUnitIndices[unitIdx];
					continue;
				}

				// 7. Fine-grained Check (Squared Distance)
				float dx = u->x - centerX;
				float dy = u->y - centerY;
				float distSq = dx * dx + dy * dy;

				if (distSq <= radiusSq) {
					if (count < maxResults) {
						results[count++] = u->id;
						alreadyProcessed[unitHandleIndex] = TRUE;
					} else {
						return count;
					}
				}

				unitIdx = gameState->nextUnitIndices[unitIdx];
			}
		}
	}

	return count;
}

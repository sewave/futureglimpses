#include "spatial.h"

static int buckets[BOARD_SIZE];
static int nextUnitIndices[MAX_GAME_UNITS];

/**
 * Converts unordered world coordinates (x1, y1, x2, y2) passed by pointer 
 * into clamped, ordered grid indices (GridRect).
 * * IMPORTANT: This function modifies the inputs pointed to by x1In, y1In, x2In, y2In 
 * to store the clamped, ordered (min/max) values upon return.
 * * @param x1In Pointer to the first X coordinate (will hold xMin on return).
 * @param y1In Pointer to the first Y coordinate (will hold yMin on return).
 * @param x2In Pointer to the second X coordinate (will hold xMax on return).
 * @param y2In Pointer to the second Y coordinate (will hold yMax on return).
 * @return GridRect The clamped grid cell boundaries for iteration.
 */
static GridRect world_to_grid_rect(float *x1In, float *y1In, float *x2In, float *y2In) {
	GridRect gridRect;
	float temp;

	// 1. Normalize/Order: Ensure x1In/y1In point to min and x2In/y2In point to max
	if (*x1In > *x2In) {
		temp = *x1In;
		*x1In = *x2In;
		*x2In = temp;
	}
	if (*y1In > *y2In) {
		temp = *y1In;
		*y1In = *y2In;
		*y2In = temp;
    }
	// x1In now points to xMin, x2In points to xMax, etc.

	// 2. Clamp World Coordinates to World Bounds [0, WORLD_WIDTH - 1] x [0, WORLD_HEIGHT - 1]
	if (*x1In < 0) *x1In = 0;
	if (*y1In < 0) *y1In = 0;
	if (*x2In >= WORLD_WIDTH) *x2In = WORLD_WIDTH - 1;
	if (*y2In >= WORLD_HEIGHT) *y2In = WORLD_HEIGHT - 1;

	// 3. Convert Clamped World Coordinates to Grid Indices
	gridRect.startCol = (int) (*x1In * INVERSE_TILE_SIZE);
	gridRect.endCol = (int) (*x2In * INVERSE_TILE_SIZE);
	gridRect.startRow = (int) (*y1In * INVERSE_TILE_SIZE);
	gridRect.endRow = (int) (*y2In * INVERSE_TILE_SIZE);

	// 4. Clamp Grid Indices to Grid Bounds [0, BOARD_WIDTH-1] x [0, BOARD_HEIGHT-1]
	if (gridRect.endCol >= BOARD_WIDTH) gridRect.endCol = BOARD_WIDTH - 1;
	if (gridRect.endRow >= BOARD_HEIGHT) gridRect.endRow = BOARD_HEIGHT - 1;

	return gridRect;
}

void spatial_clear_grid(GameContext *gameContext) {
	memset(buckets, UNUSED_BUCKET_ID, sizeof(buckets));
}

void spatial_register_unit(GameContext *gameContext, GameUnit *unit) {
	int index = GET_INDEX(unit->id); 
    
    // Calculate unordered world bounds (x1, y1, x2, y2)
    float x1 = unit->x - unit->width / 2.0f;
    float y1 = unit->y - unit->height / 2.0f;
    float x2 = unit->x + unit->width / 2.0f;
    float y2 = unit->y + unit->height / 2.0f;

    // Call with addresses of coordinates. x1/x2/y1/y2 are now min/max clamped upon return.
    GridRect bounds = world_to_grid_rect(&x1, &y1, &x2, &y2);

    for (int r = bounds.startRow; r <= bounds.endRow; r++) {
        for (int c = bounds.startCol; c <= bounds.endCol; c++) {
            
            int cellIndex = r * BOARD_WIDTH + c;

            // Prepend to the linked list
            nextUnitIndices[index] = buckets[cellIndex];
            buckets[cellIndex] = index;
        }
    }
}

// Area Query: Supports Contextual (sourceHandle > 0) and Independent (sourceHandle == 0) queries.
int spatial_get_units_in_area(float x1In, float y1In, float x2In, float y2In, int* results, int maxResults, 
                      GenericQueryFilterFunc filterFunc, int sourceHandle, GameContext* context) {
    int count = 0;

    float xMinWorld = x1In; 
    float xMaxWorld = x2In; 
    float yMinWorld = y1In; 
    float yMaxWorld = y2In; 
    
    // 1. Calculate Grid Bounds using unordered inputs. 
    // world_to_grid_rect mutates x1In/y1In/x2In/y2In to be the clamped, ordered (min/max) values.
    GridRect bounds = world_to_grid_rect(&xMinWorld, &yMinWorld, &xMaxWorld, &yMaxWorld);

    unsigned char alreadyProcessed[MAX_GAME_UNITS] = { FALSE }; 
    GameUnit* sourceUnit = game_unit_get_by_handle(context, sourceHandle); 

    for (int r = bounds.startRow; r <= bounds.endRow; r++) {
        for (int c = bounds.startCol; c <= bounds.endCol; c++) {
            
            int cellIndex = r * BOARD_WIDTH + c;
            int unitIdx = buckets[cellIndex];

            while (unitIdx != -1) {
                GameUnit* candidate = &context->units[unitIdx];
                int candidateHandleIndex = GET_INDEX(candidate->id);

                if (alreadyProcessed[candidateHandleIndex]) {
                    unitIdx = nextUnitIndices[unitIdx];
                    continue;
                }
                
                // FILTER CHECK: Context is the first argument.
                if (filterFunc && !filterFunc(context, candidate, sourceUnit)) {
                    unitIdx = nextUnitIndices[unitIdx];
                    continue;
                }
                
                // Fine-grained center-point visibility check (AABB) using the clamped, ordered coordinates
                // Checks if the unit's center point is within the query box.
                if (candidate->x >= xMinWorld && candidate->x <= xMaxWorld && 
                    candidate->y >= yMinWorld && candidate->y <= yMaxWorld) {
                    
                    if (count < maxResults) {
                        results[count++] = candidate->id; 
                        alreadyProcessed[candidateHandleIndex] = TRUE;
                    } else {
                        return count; 
                    }
                }
                unitIdx = nextUnitIndices[unitIdx];
            }
        }
    }
    return count;
}

// Radius Query: Supports Contextual (sourceHandle > 0) and Independent (sourceHandle == 0) queries.
int spatial_get_units_in_radius(float centerX, float centerY, float radius, int* results, int maxResults, 
                        GenericQueryFilterFunc filterFunc, int sourceHandle, GameContext* context) {
    int count = 0;
    float radiusSq = radius * radius;
    
    // Calculate the World Bounding Box (AABB) of the radius (unordered endpoints)
    float x1 = centerX - radius;
    float y1 = centerY - radius;
    float x2 = centerX + radius;
    float y2 = centerY + radius;
    
    // Call with addresses. world_to_grid_rect handles ordering, clamping, and updates x1/x2/y1/y2.
    GridRect bounds = world_to_grid_rect(&x1, &y1, &x2, &y2);

    // Note: The modified x1/x2/y1/y2 variables hold the clamped world AABB, which is only used 
    // for calculating the grid bounds, not the fine-grained distance check below.

    unsigned char alreadyProcessed[MAX_GAME_UNITS] = { FALSE }; 
    GameUnit* sourceUnit = game_unit_get_by_handle(context, sourceHandle); 

    for (int r = bounds.startRow; r <= bounds.endRow; r++) {
        for (int c = bounds.startCol; c <= bounds.endCol; c++) {
            
            int cellIndex = r * BOARD_WIDTH + c;
            int unitIdx = buckets[cellIndex];

            while (unitIdx != -1) {
                GameUnit* candidate = &context->units[unitIdx];
                int candidateHandleIndex = GET_INDEX(candidate->id);

                if (alreadyProcessed[candidateHandleIndex]) {
                    unitIdx = nextUnitIndices[unitIdx];
                    continue;
                }
                
                // FILTER CHECK
                if (filterFunc && !filterFunc(context, candidate, sourceUnit)) {
                    unitIdx = nextUnitIndices[unitIdx];
                    continue;
                }
                
                // Fine-grained distance check (distance from center to unit center)
                float dx = candidate->x - centerX;
                float dy = candidate->y - centerY;
                float distSq = dx*dx + dy*dy;
                
                if (distSq <= radiusSq) {
                    if (count < maxResults) {
                        results[count++] = candidate->id; 
                        alreadyProcessed[candidateHandleIndex] = TRUE;
                    } else {
                        return count; 
                    }
                }
                unitIdx = nextUnitIndices[unitIdx];
            }
        }
    }

    return count;
}

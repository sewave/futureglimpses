#include "spatial.h"

void spatial_clear_grid(GameState* gameState) {
    memset(gameState->buckets, UNUSED_BUCKET_ID, sizeof(gameState->buckets));
}

void spatial_register_unit(GameState* gameState, GameUnit* u) {
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
    int startCol = (int)(xMin * INVERSE_TILE_SIZE);
    int endCol   = (int)(xMax * INVERSE_TILE_SIZE);
    int startRow = (int)(yMin * INVERSE_TILE_SIZE);
    int endRow   = (int)(yMax * INVERSE_TILE_SIZE);

    // Clamp grid indices
    if (startCol < 0) startCol = 0;
    if (startRow < 0) startRow = 0;
    if (endCol >= BOARD_WIDTH) endCol = BOARD_WIDTH - 1;
    if (endRow >= BOARD_HEIGHT) endRow = BOARD_HEIGHT - 1;

    int index = GET_INDEX(u->id);
    // Iterate and register the unit in ALL overlapping cells
    for (int r = startRow; r <= endRow; r++) {
        for (int c = startCol; c <= endCol; c++) {
            int cellIndex = r * BOARD_WIDTH + c;
            gameState->nextUnitIndices[index] = gameState->buckets[cellIndex];
            gameState->buckets[cellIndex] = index;
        }
    }
}

int spatial_get_nearby_units(GameState* gameState, GameUnit* me, int* results, int maxResults) {
    int count = 0;
    int myCol = (int)(me->x * INVERSE_TILE_SIZE);
    int myRow = (int)(me->y * INVERSE_TILE_SIZE);
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

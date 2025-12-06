#include "util.h"
#include <stdlib.h>

char has_minimal_free_memory(int megaBytes) {
	void* ptr = malloc(megaBytes * 1024 * 1024);
	if (ptr) {
		free(ptr);
		return 1;
	}
	return 0;
}

int distance_sq(int x1, int y1, int x2, int y2) {
	int xDif = x2 - x1;
	int yDif = y2 - y1;
	return xDif * xDif + yDif * yDif;
}

/**
 * Generates a random int between [minVal, maxVal].
 */
int random_int(int minVal, int maxVal) {
	return minVal + (rand() % (maxVal - minVal + 1));
}

#include "util.h"

char has_minimal_free_memory(int megaBytes) {
	void* ptr = malloc(megaBytes * 1024 * 1024);
	if (ptr) {
		free(ptr);
		return 1;
	}
	return 0;
}

unsigned char is_point_in_box(float px, float py, float x1, float y1, float x2, float y2) {
    return (px >= x1 && px <= x2 && py >= y1 && py <= y2);
}

#ifndef UTIL_H
#define UTIL_H
#include <stdlib.h>

char has_minimal_free_memory(int megaBytes);
unsigned char is_point_in_box(float px, float py, float x1, float y1, float x2, float y2);

#endif // UTIL_H

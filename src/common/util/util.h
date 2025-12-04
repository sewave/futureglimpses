#ifndef UTIL_H
#define UTIL_H
#include <stdlib.h>

#define min_val(a, b) ((a) < (b) ? (a) : (b))
#define max_val(a, b) ((a) > (b) ? (a) : (b))
#define clamp(val, minVal, maxVal) (max_val((minVal), min_val((val), (maxVal))))

char has_minimal_free_memory(int megaBytes);

#endif // UTIL_H

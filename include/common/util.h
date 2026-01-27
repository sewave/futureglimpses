#ifndef UTIL_H
#define UTIL_H

#define min_val(a, b) ((a) < (b) ? (a) : (b))
#define max_val(a, b) ((a) > (b) ? (a) : (b))
#define clamp(val, minVal, maxVal) (max_val((minVal), min_val((val), (maxVal))))

char has_minimal_free_memory(int megaBytes);
int distance_sq(int x1, int y1, int x2, int y2);
int random_int(int minVal, int maxVal);

#endif // UTIL_H

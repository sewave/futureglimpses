#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <time.h>
// Uncomment to compile debug mode with logs
//#define DEBUGMODE
#include <allegro/debug.h>

#ifdef DEBUGMODE
#define TRACE_START(func) clock_t __trace_start_##func = clock()
#define TRACE_END(func)                                                                                   \
	do {                                                                                                  \
		clock_t __trace_end = clock();                                                                    \
		double __trace_elapsed = ((double) (__trace_end - __trace_start_##func)); \
		TRACE("[TRACE] %s: %.3f ms\n", #func, __trace_elapsed);                                             \
	} while (0)
#else
#define TRACE_START(func)
#define TRACE_END(func)
#endif

typedef enum {
    INITIALIZATION_OK = 0,
    INITIALIZATION_ERROR = -1
} InitializationStatusEnum;

#define ALLEGRO_INIT_OK 0

#define PROGRAM_OK 0
#define PROGRAM_ERROR 1

#define FUNCTION_OK 0
#define FUNCTION_ERROR -1

typedef struct {
	int x, y;
} Position;

typedef struct {
	int width, height;
} Size;

#define RESOLUTION_MULTIPLIER 3

char common_init_basic(
		int minCpuFamily,
		int requiredCpuCapabilities,
		const char *unsupportedCpuMessage,
		int requiredRamMb,
		int (*mouse_init_func)(void));
void common_print_ok();
void common_print_ko();
void common_print_init_step();
void common_print_ok_steps();

#endif /* COMMON_H */

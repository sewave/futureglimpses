#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <time.h>
// Uncomment to compile debug mode with logs
//#define DEBUGMODE
#include <allegro/debug.h>

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

#ifdef DEBUG
#define X_RESOLUTION_MULTIPLIER 3
#define Y_RESOLUTION_MULTIPLIER 3.6
#else
#define X_RESOLUTION_MULTIPLIER 2
#define Y_RESOLUTION_MULTIPLIER 2.4
 #endif

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

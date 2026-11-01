#ifndef COMMON_H
#define COMMON_H

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

// Comment to disable cheats in game
#define CHEATS_ENABLED

// Uncomment for high resolution mode
//#define HIGH_RESOLUTION

#ifdef HIGH_RESOLUTION
	#define HR_PATH "hr/"
#else
	#define HR_PATH ""
#endif

#ifdef DEBUG
	#ifdef HIGH_RESOLUTION
		#define X_RESOLUTION_MULTIPLIER 2
		#define Y_RESOLUTION_MULTIPLIER 2
	#else
		#define X_RESOLUTION_MULTIPLIER 3
		#define Y_RESOLUTION_MULTIPLIER 3.6
	#endif
#else
	#ifdef HIGH_RESOLUTION
		#define X_RESOLUTION_MULTIPLIER 1
		#define Y_RESOLUTION_MULTIPLIER 1
	#else
		#define X_RESOLUTION_MULTIPLIER 2
		#define Y_RESOLUTION_MULTIPLIER 2.4
	#endif
 #endif

char common_init_basic(
		int minCpuFamily,
		int requiredCpuCapabilities,
		const char *unsupportedCpuMessage,
		int requiredRamMb,
		int (*mouse_init_func)(void));
void common_print_ok();
void common_print_ko();
void common_print_load_step(int x, int y);

#endif /* COMMON_H */

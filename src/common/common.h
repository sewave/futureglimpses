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

#ifndef uchar
#define uchar   unsigned char
#endif

#define RESOLUTION_MULTIPLIER 3

char common_init_basic(
		int minCpuFamily,
		int requiredCpuCapabilities,
		const char *unsupportedCpuMessage,
		int requiredRamMb,
		int (*mouse_init_func)(void));

#endif /* COMMON_H */

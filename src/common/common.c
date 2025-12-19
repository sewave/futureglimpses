#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"
#include "util/util.h"
#include <allegro/system.h>
#include <allegro/keyboard.h>
#include <allegro/mouse.h>

char common_init_basic(
		int minCpuFamily,
		int requiredCpuCapabilities,
		const char *unsupportedCpuMessage,
		int requiredRamMb,
		int (*mouse_init_func)(void)) {
	printf("Initializing common systems:\n");

	/* Init all systems */
	printf("Initializing Allegro...");
	if (allegro_init() != ALLEGRO_INIT_OK) {
		printf("KO\nError initializing Allegro.");
		return PROGRAM_ERROR;
	}
	printf("OK\n");

	// Check cpu is at least a minCpuFamily and has requiredCpuCapabilities
	printf("Checking CPU capabilities...");
	if (cpu_family < minCpuFamily || !(cpu_capabilities & requiredCpuCapabilities)) {
		printf(unsupportedCpuMessage);
		return PROGRAM_ERROR;
	}
	printf("OK\n");

	// Check for requiredRamMb MB of extra memory
	printf("Checking available memory...");
	if (!has_minimal_free_memory(requiredRamMb)) {
		printf("KO\nError: Not enough memory. At least %d MB of RAM is required.", requiredRamMb);
		return PROGRAM_ERROR;
	}
	printf("OK\n");

	printf("Initializing keyboard...");
	if (install_keyboard() != ALLEGRO_INIT_OK) {
		printf("KO\nError initializing keyboard.");
		return PROGRAM_ERROR;
	}
	printf("OK\n");

	printf("Initializing mouse...");
	if (mouse_init_func != NULL) {
		if (mouse_init_func() != INITIALIZATION_OK) {
			printf("KO\nError initializing mouse.");
			return PROGRAM_ERROR;
		}
	}
	printf("OK\n");

	printf("Seeding random number generator...");
	srand(time(NULL));
	printf("OK\n");

	return PROGRAM_OK;
}

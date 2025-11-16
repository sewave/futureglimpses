#include <stdio.h>
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

	/* Init all systems */
	if (allegro_init() != ALLEGRO_INIT_OK) {
		printf("Error initializing Allegro.");
		return PROGRAM_ERROR;
	}
	// Check cpu is at least a 486 and has fpu
	if (cpu_family < minCpuFamily || !(cpu_capabilities & requiredCpuCapabilities)) {
		printf(unsupportedCpuMessage);
		return PROGRAM_ERROR;
	}
	// Check for 8 MB of memory
	if (!has_minimal_free_memory(requiredRamMb)) {
		printf("Error: Not enough memory. At least %d MB of RAM is required.", requiredRamMb);
		return PROGRAM_ERROR;
	}
	if (install_keyboard() != ALLEGRO_INIT_OK) {
		printf("Error initializing keyboard.");
		return PROGRAM_ERROR;
	}

	if (mouse_init_func != NULL) {
		if (mouse_init_func() != INITIALIZATION_OK) {
			printf("Error initializing mouse.");
			return PROGRAM_ERROR;
		}
	}

	return PROGRAM_OK;
}
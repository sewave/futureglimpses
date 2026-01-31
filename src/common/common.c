#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common/common.h"
#include "common/util.h"
#include <allegro.h>

char common_init_basic(
		int minCpuFamily,
		int requiredCpuCapabilities,
		const char *unsupportedCpuMessage,
		int requiredRamMb,
		int (*mouse_init_func)(void)) {
	printf("\n***Initializing common systems***\n");

	/* Init all systems */
	printf("Initializing Allegro...");
	if (allegro_init() != ALLEGRO_INIT_OK) {
		common_print_ko();
		printf("Error initializing Allegro.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	// Check cpu is at least a minCpuFamily and has requiredCpuCapabilities
	printf("Checking CPU capabilities...");
	if (cpu_family < minCpuFamily || (requiredCpuCapabilities && !(cpu_capabilities & requiredCpuCapabilities))) {
		common_print_ko();
		printf(unsupportedCpuMessage);
		return PROGRAM_ERROR;
	}
	common_print_ok();

	// Check for requiredRamMb MB of extra memory
	printf("Checking required memory (%d MB)...", requiredRamMb);
	if (!has_minimal_free_memory(requiredRamMb)) {
		common_print_ko();
		printf("Error: Not enough memory. At least %d MB of RAM is required.", requiredRamMb);
		return PROGRAM_ERROR;
	}
	common_print_ok();

	printf("Initializing keyboard...");
	if (install_keyboard() != ALLEGRO_INIT_OK) {
		common_print_ko();
		printf("Error initializing keyboard.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	printf("Initializing mouse [");
	if (mouse_init_func != NULL) {
		if (mouse_init_func() != INITIALIZATION_OK) {
			common_print_ko();
			printf("Error initializing mouse.");
			return PROGRAM_ERROR;
		}
	}
	common_print_ok_steps();

	printf("Seeding random number generator...");
	srand(time(NULL));
	common_print_ok();

	printf("***Common systems initialized***\n\n");

	return PROGRAM_OK;
}

void common_print_ok() {
	printf("OK\n");
}

void common_print_ok_steps() {
	printf("] OK\n");
}

void common_print_ko() {
	printf("KO\n");
}

void common_print_init_step() {
	printf("*");
	fflush(stdout);
}
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro.h>
#include "common/common.h"
#include "common/util.h"
#include "common/console.h"

#define LOAD_STATES 8
static int loadState = 0;
static char loadStateChars[] = { '|', '/', '-', '\\', '|', '/', '-', '\\'  };

static void common_set_print_style() {
	console_set_text_color(CONSOLE_COLOR_LIGHT_GRAY);
	console_set_background_color(CONSOLE_COLOR_BLACK);
	console_set_blink_state(CONSOLE_BLINK_OFF);
}

char common_init_basic(
		int minCpuFamily,
		int requiredCpuCapabilities,
		const char *unsupportedCpuMessage,
		int requiredRamMb,
		int (*mouse_init_func)(void)) {
	console_set_text_color(CONSOLE_COLOR_YELLOW);
	console_set_background_color(CONSOLE_COLOR_RED);
	console_printf("\r\n     Initializing common systems    \r\n");
	common_set_print_style();

	/* Init all systems */
	printf("Initializing Allegro..............");
	if (allegro_init() != ALLEGRO_INIT_OK) {
		common_print_ko();
		console_printf("Error initializing Allegro.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	// Check cpu is at least a minCpuFamily and has requiredCpuCapabilities
	printf("Checking CPU capabilities.........");
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

	printf("Initializing keyboard.............");
	if (install_keyboard() != ALLEGRO_INIT_OK) {
		common_print_ko();
		printf("Error initializing keyboard.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	printf("Initializing mouse................");
	if (mouse_init_func != NULL) {
		if (mouse_init_func() != INITIALIZATION_OK) {
			common_print_ko();
			console_printf("Error initializing mouse.");
			return PROGRAM_ERROR;
		}
	}
	common_print_ok();

	printf("Seeding random number generator...");
	srand(time(NULL));
	common_print_ok();

	return PROGRAM_OK;
}

void common_print_ok() {
	console_set_text_color(CONSOLE_COLOR_GREEN);
	console_set_background_color(CONSOLE_COLOR_BLACK);
	console_printf("OK\r\n");
	common_set_print_style();
}

void common_print_ko() {
	console_set_text_color(CONSOLE_COLOR_RED);
	console_set_blink_state(CONSOLE_BLINK_ON);
	console_printf("KO\r\n");
	common_set_print_style();
}

void common_print_load_step(int x, int y) {
	console_move_cursor(x, y);
	loadState = (loadState + 1) % LOAD_STATES;
	printf("%c", loadStateChars[loadState]);
	fflush(stdout);
	console_move_cursor(x, y);
}

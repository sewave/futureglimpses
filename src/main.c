#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
#include "common/common_lib.h"
#include "game/game_lib.h"

volatile int closeButtonPressed = FALSE;

void close_button_handler() {
	closeButtonPressed = TRUE;
}
END_OF_FUNCTION(close_button_handler)

#define LOGIC_RATE_BPS 60
#define MAX_CATCHUP_TICKS 5
#define PROGRAM_REQUIRED_RAM_MB 8
#define MINIMAL_CPU_FAMILY CPU_FAMILY_I486
#define REQUIRED_CPU_CAPABILITIES CPU_FPU
#define UNSUPPORTED_CPU_MESSAGE "Error: CPU not supported. A 486 or better with FPU is required."

volatile long logic_ticks = 0;

void timer_handler() {
	logic_ticks++;
}
END_OF_FUNCTION(timer_handler);

FONT *customFont = NULL;

void main_loop(BITMAP* screenBuffer, volatile long* logicTicks, volatile int* closeButtonFlag, int maxCatchUpTicks, int endState);

void install_timers() {
	/* Attach function to close button */
	LOCK_VARIABLE(closeButtonPressed);
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	/* Install timer handler function */
	LOCK_VARIABLE(logic_ticks);
	LOCK_FUNCTION(timer_handler);
	install_int_ex(timer_handler, BPS_TO_TIMER(LOGIC_RATE_BPS));
}

int main(int argc, char *argv[]) {
	// MIN_CPU, CPU_REQ, RAM_REQ, USE_MOUSE
	if (common_init_basic(
				MINIMAL_CPU_FAMILY,
				REQUIRED_CPU_CAPABILITIES,
				UNSUPPORTED_CPU_MESSAGE,
				PROGRAM_REQUIRED_RAM_MB,
				&game_mouse_init_cursors) != PROGRAM_OK) {
		return PROGRAM_ERROR;
	}

	if (video_init_system(GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, GAME_COLOR_DEPTH) != INITIALIZATION_OK) {
		printf("Error initializing video.");
		return PROGRAM_ERROR;
	}

	if (snd_init_system(GAME_VOICES, MOD_VOICES, MUSIC_TYPE_MOD) != INITIALIZATION_OK) {
		printf("Error initializing sound.");
		return PROGRAM_ERROR;
	}

	install_timers();

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	set_mouse_sprite_focus(0, 0);

	game_snd_load_sounds();

	BITMAP *screenBuffer = create_bitmap(GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);

	customFont = load_font("assets/font/ex01.pcx", NULL, NULL);

	PALETTE p;
	if(video_load_raw_palette("assets/pal/game.pal", p) != PROGRAM_OK) {
		return PROGRAM_ERROR;
	}
	set_palette_range(p, 32, 247, 0);

	fps_init();
	// mod music uses 5 FPS
	//game_snd_play_music(GAME_MUSIC_TITLE);
	globalGameState.gameState = GAME_STATE_LOAD_MAP;
	main_loop(screenBuffer, &logic_ticks, &closeButtonPressed, MAX_CATCHUP_TICKS, GAME_STATE_EXIT);

	snd_stop_music();
	snd_destroy_sounds();
	mouse_destroy_cursors();
	destroy_bitmap(screenBuffer);
	destroy_font(customFont);
	game_free_game_state(&globalGameState);
	allegro_exit();
	return PROGRAM_OK;
}

static long lastTickCount;
static char redrawNeeded;
static RenderQueue renderQueue;
static char keyPrevious[KEY_MAX];
char fpsText[16];

void main_loop(BITMAP *screenBuffer,
			   volatile long *logicTicks,
			   volatile int *closeButtonFlag,
			   int maxCatchUpTicks,
			   int endState) {
	memset(keyPrevious, FALSE, sizeof(keyPrevious));
	lastTickCount = *logicTicks;
	redrawNeeded = FALSE;
	render_queue_init(&renderQueue);
	while (!*closeButtonFlag && globalGameState.gameState != endState) {
		if (*logicTicks > lastTickCount) {
			long ticksToCatchup = *logicTicks - lastTickCount;
			if (ticksToCatchup > maxCatchUpTicks) {
				lastTickCount = *logicTicks - maxCatchUpTicks;
				ticksToCatchup = maxCatchUpTicks;
			}
			while (ticksToCatchup > 0) {
				render_queue_clear(&renderQueue);
				memcpy(keyPrevious, (char *) key, sizeof(keyPrevious));
				poll_keyboard();
				poll_mouse();
				globalGameState.gameState = gameStateTable[globalGameState.gameState](&globalGameState, &renderQueue);
				lastTickCount++;
				ticksToCatchup--;
			}
			redrawNeeded = TRUE;
		}

		// Render game
		if (redrawNeeded) {
			render_queue_execute(&renderQueue, screenBuffer);
			vsync();
			acquire_screen();
			#ifdef DOS
				blit(screenBuffer, screen, 0, 0, 0, 0, screen->w, screen->h);
			#else
				stretch_blit(screenBuffer, screen, 0, 0, screenBuffer->w, screenBuffer->h, 0, 0, screen->w, screen->h);
			#endif
			release_screen();
			redrawNeeded = FALSE;
			fps_update();
		} else {
			//rest(1);
		}
	}
}

END_OF_MAIN()

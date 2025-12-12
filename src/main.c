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

volatile long logic_ticks = 0;

void timer_handler() {
	logic_ticks++;
}
END_OF_FUNCTION(timer_handler);

FONT *customFont = NULL;

void main_loop(volatile long* logicTicks, volatile int* closeButtonFlag, int maxCatchUpTicks, int endState);

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

	if (snd_init_system(GAME_VOICES, MOD_VOICES, MUSIC_TYPE_MOD) != INITIALIZATION_OK) {
		printf("Error initializing sound. Continuing without sound.");
	}

	install_timers();

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	set_mouse_sprite_focus(0, 0);

	game_snd_load_sounds();
	if(game_gfx_load_sprite_sheets() != INITIALIZATION_OK) {
		game_gfx_destroy_sprite_sheets();
		printf("Error loading sprite sheets.");
		return PROGRAM_ERROR;
	}

	if (video_init_system(GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, GAME_COLOR_DEPTH) != INITIALIZATION_OK) {
		printf("Error initializing video.");
		return PROGRAM_ERROR;
	}

	if (game_video_load_universal_pal() != INITIALIZATION_OK) {
		printf("Error loading game pal.");
		return PROGRAM_ERROR;
	}

	fps_init();
	// mod music uses ~5 FPS
	// game_snd_play_music(GAME_MUSIC_TITLE);

	main_loop(&logic_ticks, &closeButtonPressed, MAX_CATCHUP_TICKS, GAME_STATE_EXIT);

	game_gfx_destroy_sprite_sheets();
	snd_stop_music();
	snd_destroy_sounds();
	mouse_destroy_cursors();
	allegro_exit();
	return PROGRAM_OK;
}
END_OF_MAIN()

static long lastTickCount;
static char redrawNeeded;
static RenderQueue renderQueue;
static char keyPrevious[KEY_MAX];
static GameContext context;

void main_loop(volatile long *logicTicks,
			   volatile int *closeButtonFlag,
			   int maxCatchUpTicks,
			   int endState) {
	BITMAP *screenBuffer = create_bitmap(GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);
	context.gameState = GAME_STATE_LOAD_MAP;
	memset(keyPrevious, FALSE, sizeof(keyPrevious));
	redrawNeeded = FALSE;
	render_queue_init(&renderQueue);
	lastTickCount = *logicTicks;
	mouse_initialize_status(&(context.mouseStatus));
	while (!*closeButtonFlag && context.gameState != endState) {
		if (*logicTicks > lastTickCount) {
			context.ticksToCatchup = *logicTicks - lastTickCount;
			if (context.ticksToCatchup > maxCatchUpTicks) {
				lastTickCount = *logicTicks - maxCatchUpTicks;
				context.ticksToCatchup = maxCatchUpTicks;
			}
			while (context.ticksToCatchup > 0) {
				context.ticksToCatchup--;
				render_queue_clear(&renderQueue);
				memcpy(keyPrevious, (char *) key, sizeof(keyPrevious));
				poll_keyboard();
				mouse_update_status(&context.mouseStatus);
				context.gameState = game_execute_state(&context, &renderQueue);
				lastTickCount++;
			}
			redrawNeeded = TRUE;
		}
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
		}
	}
	destroy_bitmap(screenBuffer);
	game_free_context(&context);
}

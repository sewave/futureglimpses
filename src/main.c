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

static long lastTickCount;
static char redrawNeeded;
static RenderQueue renderQueue;
static GameContext context;

void main_loop(volatile long* logicTicks, volatile int* closeButtonFlag, int maxCatchUpTicks, int endState);

void install_timers() {
	printf("Installing timers...");
	/* Attach function to close button */
	LOCK_VARIABLE(closeButtonPressed);
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	/* Install timer handler function */
	LOCK_VARIABLE(logic_ticks);
	LOCK_FUNCTION(timer_handler);
	install_int_ex(timer_handler, BPS_TO_TIMER(LOGIC_RATE_BPS));
	printOK();
}

int main(int argc, char *argv[]) {
	printf("Starting %s v%s...\n", GAME_TITLE, VERSION);
	// MIN_CPU, CPU_REQ, RAM_REQ, USE_MOUSE
	if (common_init_basic(MINIMAL_CPU_FAMILY, REQUIRED_CPU_CAPABILITIES,
				UNSUPPORTED_CPU_MESSAGE, PROGRAM_REQUIRED_RAM_MB,
				&game_mouse_init_cursors) != PROGRAM_OK) {
		return PROGRAM_ERROR;
	}
	set_window_title(GAME_TITLE);

	game_config_load_settings(&context.config);

	if (snd_init_system(GAME_VOICES, MOD_VOICES, MUSIC_TYPE_MOD) != INITIALIZATION_OK) {
		printKO();
		printf("Error initializing sound. Continuing without sound.");
	}

	install_timers();

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

	game_snd_load_sounds();
	set_volume(context.config.sfxVolume, context.config.musicVolume);
	
	if(game_gfx_load_sprite_sheets() != INITIALIZATION_OK) {
		game_gfx_destroy_sprite_sheets();
		printKO();
		printf("Error loading sprite sheets.\n");
		return PROGRAM_ERROR;
	}

	if (game_text_init_system(context.config.language) != INITIALIZATION_OK) {
		printKO();
		printf("Error initializing text system.");
		return PROGRAM_ERROR;
	}
	printOK();

	printf("Loading game font...");
	context.gameFont = load_font("assets/font/main.pcx", NULL, NULL);
	if (context.gameFont == NULL) {
		printKO();
		printf("Error loading game font.");
		return PROGRAM_ERROR;
	}
	printOK();

	printf("\n***Press any key/Pulsa cualquier tecla***\n\n");
	while (keypressed()) {
		readkey();
	}
	readkey();

	if (video_init_system(GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, GAME_COLOR_DEPTH) != INITIALIZATION_OK) {
		printKO();
		printf("Error initializing video (%d, %d, %d).", GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, GAME_COLOR_DEPTH);
		return PROGRAM_ERROR;
	}

	if (game_video_load_universal_pal() != INITIALIZATION_OK) {
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		printf("Error loading universal game pal.");
		return PROGRAM_ERROR;
	}

	fps_init();
	// mod music uses ~5 FPS
	// game_snd_play_music(GAME_MUSIC_TITLE);

	printf("\n***Starting game loop***\n\n");

	main_loop(&logic_ticks, &closeButtonPressed, MAX_CATCHUP_TICKS, GAME_STATE_EXIT);

	game_gfx_destroy_sprite_sheets();
	snd_stop_music();
	snd_destroy_sounds();
	game_mouse_destroy_cursors();
	text_free_all();
	game_config_save_settings(&context.config);
	allegro_exit();
	return PROGRAM_OK;
}
END_OF_MAIN()

void main_loop(volatile long *logicTicks,
			   volatile int *closeButtonFlag,
			   int maxCatchUpTicks,
			   int endState) {
	BITMAP *screenBuffer = create_bitmap(GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);
	context.gameState = GAME_STATE_LOAD_MAP;
	redrawNeeded = FALSE;
	render_queue_init(&renderQueue);
	lastTickCount = *logicTicks;
	mouse_initialize_status(&context.mouseStatus, SEC_TO_FRAMES(0.3f));
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
				keyboard_update();
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

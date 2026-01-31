#include <stdio.h>
#include <allegro.h>
#include "game/game_lib.h"

static RenderQueue renderQueue;
static GameContext context;

int main_init() {
    printf("Starting %s v%s...\n", GAME_TITLE, VERSION);
	// MIN_CPU, CPU_REQ, RAM_REQ, USE_MOUSE
	if (common_init_basic(MINIMAL_CPU_FAMILY, REQUIRED_CPU_CAPABILITIES,
				UNSUPPORTED_CPU_MESSAGE, PROGRAM_REQUIRED_RAM_MB,
				&game_mouse_init_cursors) != PROGRAM_OK) {
		return PROGRAM_ERROR;
	}
	set_window_title(GAME_TITLE);

	game_config_load_settings(&context.config);

	if (snd_init_system(GAME_VOICES, MOD_VOICES, MUSIC_TYPE_MIDI) != INITIALIZATION_OK) {
		common_print_ko();
		printf("Error initializing sound. Continuing without sound.");
	}

	printf("Installing interruptions...");
	close_install_handler();
	timer_init(LOGIC_RATE_BPS);
	common_print_ok();

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

	game_snd_load_sounds();
	set_volume(context.config.sfxVolume, context.config.musicVolume);

	if (game_gfx_load_all() != INITIALIZATION_OK) {
		game_gfx_destroy_all();
		common_print_ok_steps();
		printf("Error loading gfx.\n");
		return PROGRAM_ERROR;
	}

	if (game_text_init_system(context.config.language) != INITIALIZATION_OK) {
		common_print_ko();
		printf("Error initializing text system.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	printf("Loading game font...");
	context.gameFont = load_font("assets/font/bitrimus.pcx", NULL, NULL);
	if (context.gameFont == NULL) {
		common_print_ko();
		printf("Error loading game font.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	printf("\n***Press any key/Pulsa cualquier tecla***\n\n");
	while (keypressed()) {
		readkey();
	}
	readkey();

	if (video_init_system(GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, GAME_COLOR_DEPTH) != INITIALIZATION_OK) {
		common_print_ko();
		printf("Error initializing video (%d, %d, %d).", GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, GAME_COLOR_DEPTH);
		return PROGRAM_ERROR;
	}

	if (game_video_load_universal_pal() != INITIALIZATION_OK) {
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		printf("Error loading universal game pal.");
		return PROGRAM_ERROR;
	}

	fps_init();
	context.screenBuffer = create_bitmap(GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);
	context.gameState = GAME_STATE_INIT_TITLE;
	render_queue_init(&renderQueue);
	mouse_initialize_status(&context.mouseStatus, SEC_TO_FRAMES(0.3f));

	return PROGRAM_OK;
}

void main_loop() {
	uint8_t redrawNeeded = FALSE;
	while (!close_is_pressed() && context.gameState != GAME_STATE_EXIT) {
		if (timer_has_ticks()) {
			context.ticksToCatchup = timer_get_ticks();
			if (context.ticksToCatchup > MAX_CATCHUP_TICKS) context.ticksToCatchup = MAX_CATCHUP_TICKS;
			timer_reset_ticks();
			while (context.ticksToCatchup > 0) {
				context.ticksToCatchup--;
				render_queue_clear(&renderQueue);
				context.gameState = game_execute_state(&context, &renderQueue);
				keyboard_update();
				mouse_update_status(&context.mouseStatus);
			}
			redrawNeeded = TRUE;
		}
		if (redrawNeeded) {
			render_queue_execute(&renderQueue, context.screenBuffer);
			vsync();
			acquire_screen();
			#ifdef DOS
				blit(context.screenBuffer, screen, 0, 0, 0, 0, screen->w, screen->h);
			#else
				stretch_blit(context.screenBuffer, screen, 0, 0, context.screenBuffer->w, context.screenBuffer->h, 0, 0, screen->w, screen->h);
			#endif
			release_screen();
			redrawNeeded = FALSE;
			fps_update();
		}
	}
}

void main_clean() {
	game_free_context(&context);
	game_gfx_destroy_all();
	snd_stop_music();
	snd_destroy_sounds();
	game_mouse_destroy_cursors();
	text_free_all();
	game_config_save_settings(&context.config);
	allegro_exit();
}

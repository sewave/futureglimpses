#include <stdio.h>
#include "common/console.h"
#include "game/game.h"
#include "game/video/gfx.h"
#include "game/video/game_video.h"
#include "game/mouse/game_mouse.h"
#include "game/sound/game_sound.h"
#include "game/config/config.h"
#include "game/text/game_text.h"

static RenderQueue renderQueue;
static GameContext context;

static void get_minimap_colors(GameContext *context) {
	BITMAP *tilesetColors = game_gfx_get_tileset_colors();
	for (int x = 0; x < TILESET_TILES_COLOR_WIDTH; x++) {
		for (int y = 0; y < TILESET_TILES_COLOR_HEIGHT; y++) {
			context->minimapColors[x + y * TILESET_TILES_COLOR_WIDTH] = getpixel(tilesetColors, x, y);
		}
	}
}

int main_init() {
	console_set_text_color(CONSOLE_COLOR_LIGHT_GRAY);
	console_set_background_color(CONSOLE_COLOR_BLACK);
	console_clear_screen();
	console_set_text_color(CONSOLE_COLOR_YELLOW);
	console_set_background_color(CONSOLE_COLOR_BLUE);
	console_print_box(0, 0, 80, 3);
	console_move_cursor(24, 1);
	printf("%s v%s, 2026 Wave\n", GAME_TITLE, VERSION);
	// MIN_CPU, CPU_REQ, RAM_REQ, USE_MOUSE
	if (common_init_basic(MINIMAL_CPU_FAMILY, REQUIRED_CPU_CAPABILITIES,
				UNSUPPORTED_CPU_MESSAGE, PROGRAM_REQUIRED_RAM_MB,
				&game_mouse_init_cursors) != PROGRAM_OK) {
		return PROGRAM_ERROR;
	}
	set_window_title(GAME_TITLE);

	console_set_text_color(CONSOLE_COLOR_YELLOW);
	console_set_background_color(CONSOLE_COLOR_RED);
	console_printf("      Initializing game systems     \r\n");
	console_reset_styles();
	game_config_load_settings(&context.config);

	if (snd_init_system(GAME_VOICES, MOD_VOICES, MUSIC_TYPE_MIDI) != INITIALIZATION_OK) {
		common_print_ko();
		printf("Error initializing sound. Continuing without sound.\n");
	}

	printf("Initializing interruptions........");
	close_install_handler();
	timer_init(LOGIC_RATE_BPS);
	common_print_ok();

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

	printf("Initializing game font............");
	context.gameFont = load_font("assets/font/bitrimus.pcx", NULL, NULL);
	if (context.gameFont == NULL) {
		common_print_ko();
		printf("Error loading game font.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	if (game_text_init_system(context.config.language) != INITIALIZATION_OK) {
		common_print_ko();
		printf("Error initializing text system.");
		return PROGRAM_ERROR;
	}
	common_print_ok();

	console_set_text_color(CONSOLE_COLOR_YELLOW);
	console_set_background_color(CONSOLE_COLOR_RED);
	console_printf("       Loading game resources       \r\n");
	console_reset_styles();

	game_snd_load_sounds();
	set_volume(context.config.sfxVolume, context.config.musicVolume);

	if (game_gfx_load_all() != INITIALIZATION_OK) {
		game_gfx_destroy_all();
		common_print_ok_steps();
		printf("Error loading gfx.\n");
		return PROGRAM_ERROR;
	}

	console_set_text_color(CONSOLE_COLOR_YELLOW);
	console_set_background_color(CONSOLE_COLOR_BLUE);
	console_set_blink_state(CONSOLE_BLINK_ON);
	console_move_cursor(19, 24);
	console_printf("***Press any key/Pulsa cualquier tecla***");
	console_reset_styles();
	while (keypressed()) {
		readkey();
	}
	readkey();

	if (video_init_system(GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, SCREEN_COLOR_DEPTH) != INITIALIZATION_OK) {
		common_print_ko();
		printf("Error initializing video (%d, %d, %d).", GAME_EXTERNAL_WIDTH, GAME_EXTERNAL_HEIGHT, SCREEN_COLOR_DEPTH);
		return PROGRAM_ERROR;
	}
	printf("\n");
	set_color_depth(GAME_COLOR_DEPTH);
	set_color_conversion(COLORCONV_NONE);
	if (game_video_load_universal_pal(context.mainPalette) != INITIALIZATION_OK) {
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		printf("Error loading universal game pal.\n");
		return PROGRAM_ERROR;
	}
	set_palette(black_palette);
	video_fade_reset();
	fps_init();
	context.screenBuffer = create_bitmap(GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);
	clear_bitmap(context.screenBuffer);
	context.gameState = GAME_STATE_SPLASH;
	render_queue_init(&renderQueue);
	mouse_initialize_status(&context.mouseStatus, SEC_TO_FRAMES(0.3f));
	get_minimap_colors(&context);

	return PROGRAM_OK;
}

void main_loop() {
	#ifndef DOS
		BITMAP* screenDepthBuffer = create_bitmap_ex(SCREEN_COLOR_DEPTH, GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);
	#endif
	timer_reset_ticks();
	GameStateEnum oldState = GAME_STATE_EXIT;
	do {
		if (timer_has_ticks()) {
			if (oldState != context.gameState) {
				game_execute_state_init(&context);
				timer_reset_ticks();
			}
			oldState = context.gameState;
			context.ticksToCatchup = timer_get_ticks();
			if (context.ticksToCatchup > MAX_CATCHUP_TICKS) context.ticksToCatchup = MAX_CATCHUP_TICKS;
			timer_reset_ticks();
			while (context.ticksToCatchup > 0) {
				context.ticksToCatchup--;
				render_queue_clear(&renderQueue);
				context.gameState = game_execute_state_update(&context);
				keyboard_update();
				mouse_update_status(&context.mouseStatus);
				if(oldState != context.gameState) break;
			}
			if (oldState == context.gameState) game_execute_state_render(&context, &renderQueue);
		}
		if (renderQueue.count > 0) {
			render_queue_execute(&renderQueue, context.screenBuffer);
			vsync();
			acquire_screen();
			#ifdef DOS
				blit(context.screenBuffer, screen, 0, 0, 0, 0, screen->w, screen->h);
			#else
				blit(context.screenBuffer, screenDepthBuffer, 0, 0, 0, 0, context.screenBuffer->w, context.screenBuffer->h);
				stretch_blit(screenDepthBuffer, screen, 0, 0, screenDepthBuffer->w, screenDepthBuffer->h, 0, 0, screen->w, screen->h);
			#endif
			release_screen();
			fps_update();
		}
		video_fade_handle();
	} while (!close_is_pressed() && context.gameState != GAME_STATE_EXIT);
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

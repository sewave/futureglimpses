#include "game/state/splash.h"
#include <allegro.h>

#define SPLASH_BACKGROUND_PATH "assets/gfx/ui/back/wave.pcx"
#define SPLASH_MASK_PATH "assets/gfx/ui/back/wavesoft.pcx"

static const int SPLASH_DURATION_TICKS = 2 * LOGIC_RATE_BPS;
static const int AFTER_SPLASH_DURATION_TICKS = 2 * LOGIC_RATE_BPS;
static int splashTicks;
static BITMAP *splashBackground;
static BITMAP *splashMask;
static int splashX, splashY;

static void calculate_splash_position() {
    float t = (float) splashTicks / SPLASH_DURATION_TICKS;
    splashX = (int) ((1 - t) * (GAME_INTERNAL_WIDTH - splashBackground->w));
    splashY = (int) ((1 - t) * (GAME_INTERNAL_HEIGHT));
}

void game_state_splash_init(GameContext *context) {
	splashTicks = 0;
	splashBackground = load_bitmap(SPLASH_BACKGROUND_PATH, NULL);
	if (!splashBackground) {
		TRACE("Failed to load splash background bitmap from path: %s\n", SPLASH_BACKGROUND_PATH);
		exit(PROGRAM_ERROR);
	}
	splashMask = load_bitmap(SPLASH_MASK_PATH, NULL);
	if (!splashMask) {
		TRACE("Failed to load splash mask bitmap from path: %s\n", SPLASH_MASK_PATH);
		exit(PROGRAM_ERROR);
	}
	calculate_splash_position();
    set_palette(context->mainPalette);
	game_snd_play_music(GAME_MUSIC_INTRO);
}

GameStateEnum game_state_splash_update(GameContext *context) {
	if (splashTicks < SPLASH_DURATION_TICKS && !keyboard_is_key_pressed(KEY_ESC)) {
		splashTicks++;
		calculate_splash_position();
        return GAME_STATE_SPLASH;
	} else {
        if (splashTicks < SPLASH_DURATION_TICKS + AFTER_SPLASH_DURATION_TICKS && !keyboard_is_key_pressed(KEY_ESC)) {
            splashTicks++;
            return GAME_STATE_SPLASH;
        }
		destroy_bitmap(splashBackground);
		splashBackground = NULL;
		destroy_bitmap(splashMask);
		splashMask = NULL;
		video_fade_out_init(DEFAULT_FADE_SPEED);
		return GAME_STATE_INTRO;
	}
}

void game_state_splash_render(GameContext *context, RenderQueue *renderQueue) {
	render_queue_submit_sprite(renderQueue, 0, splashBackground, splashX, splashY, RND_FLAG_NORMAL);
	render_queue_submit_sprite(renderQueue, 1, splashMask, 0, 0, RND_FLAG_NORMAL);
}

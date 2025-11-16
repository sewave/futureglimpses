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

GameState globalGameState;
RenderQueue renderQueue;
char keyPrevious[KEY_MAX];

#define LOGIC_RATE_BPS 60
#define MAX_CATCHUP_TICKS 5
#define PROGRAM_REQUIRED_RAM_MB 8
#define MINIMAL_CPU_FAMILY CPU_FAMILY_I486
#define REQUIRED_CPU_CAPABILITIES CPU_FPU
#define UNSUPPORTED_CPU_MESSAGE "Error: CPU not supported. A 486 or better with FPU is required."

volatile long logic_ticks = 0;
char redraw_needed = FALSE;

void timer_handler() {
    logic_ticks++;
}
END_OF_FUNCTION(timer_handler);

int mickeyx = 0;
int mickeyy = 0;
int mickeyFrames = 0;
FONT *customFont = NULL;

void queue_mouse_status(RenderQueue *queue) {
	poll_mouse();

	if (mouse_b & 1)
		render_queue_submit_text(
				queue, UI_Z_ORDER, customFont, "left button pressed", 16, 128,
				makecol(0, 0, 0), makecol(255, 255, 255));
	else
		render_queue_submit_text(
				queue, UI_Z_ORDER, customFont, "left button not pressed", 16, 128,
				makecol(0, 0, 0), makecol(255, 255, 255));

	if (mouse_b & 2)
		render_queue_submit_text(
				queue, UI_Z_ORDER, customFont, "right button pressed", 16, 144,
				makecol(0, 0, 0), makecol(255, 255, 255));
	else
		render_queue_submit_text(
				queue, UI_Z_ORDER, customFont, "right button not pressed", 16, 144,
				makecol(0, 0, 0), makecol(255, 255, 255));

	if (mouse_b & 4)
		render_queue_submit_text(
				queue, UI_Z_ORDER, customFont, "middle button pressed", 16, 160,
				makecol(0, 0, 0), makecol(255, 255, 255));
	else
		render_queue_submit_text(
				queue, UI_Z_ORDER, customFont, "middle button not pressed", 16, 160,
				makecol(0, 0, 0), makecol(255, 255, 255));
}

typedef struct {
	int x;
	int y;
	int vx;
	int vy;
} Sprite;

#define MAX_SPRITES 64

Sprite sprites[MAX_SPRITES];

void init_sprites() {
	for(int i = 0; i < MAX_SPRITES; i++) {
		sprites[i].x = rand() % GAME_INTERNAL_WIDTH;
		sprites[i].y = rand() % GAME_INTERNAL_HEIGHT;
		sprites[i].vx = (rand() % 5) + 1;
		sprites[i].vy = (rand() % 5) + 1;
	}
}

void move_sprite_and_bounce(Sprite* sprite) {
	sprite->x += sprite->vx;
	sprite->y += sprite->vy;

	if (sprite->x < 0 || sprite->x > GAME_INTERNAL_WIDTH - 16) {
		sprite->vx = -sprite->vx;
	}
	if (sprite->y < 0 || sprite->y > GAME_INTERNAL_HEIGHT - 16) {
		sprite->vy = -sprite->vy;
	}
}

void move_all_sprites() {
	for(int i = 0; i < MAX_SPRITES; i++) {
		move_sprite_and_bounce(&sprites[i]);
		render_queue_submit_sprite(&renderQueue, SPRITES_Z_ORDER + i, mouse_get_cursor_sprite(), sprites[i].x, sprites[i].y, 0);
	}
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

	/* Attach function to close button */
	LOCK_VARIABLE(closeButtonPressed);
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	/* Install timer handler function */
	LOCK_VARIABLE(logic_ticks);
	LOCK_FUNCTION(timer_handler);
	install_int_ex(timer_handler, BPS_TO_TIMER(LOGIC_RATE_BPS));

	// mod music uses 5 FPS
	//game_snd_play_music(GAME_MUSIC_TITLE);

	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	set_mouse_sprite_focus(0, 0);

	game_snd_load_sounds();

	globalGameState.gameState = GAME_STATE_MAIN_MENU;
	render_queue_init(&renderQueue);

	BITMAP *screenBuffer = create_bitmap(GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);
	memset(keyPrevious, FALSE, sizeof(keyPrevious));

	customFont = load_font("assets/font/ex01.pcx", NULL, NULL);

	fps_init();
	init_sprites();
	long last_tick_count = logic_ticks;
	while (!closeButtonPressed && !key[KEY_ESC] && globalGameState.gameState != NUM_GAME_STATES) {
		if (logic_ticks > last_tick_count) {
            long ticks_to_catchup = logic_ticks - last_tick_count;
            if (ticks_to_catchup > MAX_CATCHUP_TICKS) {
                last_tick_count = logic_ticks - MAX_CATCHUP_TICKS;
                ticks_to_catchup = MAX_CATCHUP_TICKS;
            }
            while (ticks_to_catchup > 0) {
				render_queue_clear(&renderQueue);
				memcpy(keyPrevious, (char *) key, sizeof(keyPrevious));
				poll_keyboard();
				//gameStateTable[globalGameState.gameState](&globalGameState);
				move_all_sprites();
				if (key[KEY_A] & !keyPrevious[KEY_A]) game_snd_play_sound(GAME_SOUND_SEA_WAVES);
				if (key[KEY_S] & !keyPrevious[KEY_S]) game_snd_play_sound(GAME_SOUND_CLICK);

				render_queue_submit_clear(&renderQueue, BACKGROUND_Z_ORDER, 0);
				render_queue_submit_sprite(&renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(), mouse_get_x() - mouse_x_focus, mouse_get_y() - mouse_y_focus, RND_FLAG_HV_FLIP);
				queue_mouse_status(&renderQueue);

				char fpsText[64];
				snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
				render_queue_submit_text(&renderQueue, 5000, font, fpsText, 10, 10, 1, 0);

                last_tick_count++;
                ticks_to_catchup--;
			}
			redraw_needed = TRUE; 
        }

		// Render game
		if (redraw_needed) {
			render_queue_execute(&renderQueue, screenBuffer);
			vsync();
			acquire_screen();
			stretch_blit(screenBuffer, screen, 0, 0, screenBuffer->w, screenBuffer->h, 0, 0, screen->w, screen->h);
			release_screen();
			redraw_needed = FALSE;
			fps_update();
		}
		else {
			rest(1);
		}
	}

	snd_stop_music();
	snd_destroy_sounds();
	mouse_destroy_cursors();
	destroy_bitmap(screenBuffer);
	destroy_font(customFont);
	allegro_exit();
	return PROGRAM_OK;
}

END_OF_MAIN()

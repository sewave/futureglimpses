#include <stdio.h>
#include <allegro.h>
#include <jgmod.h>
#include "common/common.h"
#include "common/sound/sound.h"
#include "common/video/video.h"
#include "common/video/render_queue.h"
#include "game/game.h"
#include "game/mouse/game_mouse.h"
#include "game/sound/game_sound.h"
#include "game/video/game_video.h"

volatile int closeButtonPressed = FALSE;

void close_button_handler() {
	closeButtonPressed = TRUE;
}
END_OF_FUNCTION(close_button_handler)

GameState globalGameState;
RenderQueue renderQueue;
char keyPrevious[KEY_MAX];

int mickeyx = 0;
int mickeyy = 0;
int mickeyFrames = 0;

void printMouse(BITMAP *buffer, RenderQueue *queue) {
	poll_mouse();

	textprintf_ex(buffer, font, 16, 48, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mouse_x = %-5d", mouse_x);
	textprintf_ex(buffer, font, 16, 64, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mouse_y = %-5d", mouse_y);

	mickeyFrames++;
	if ((mickeyFrames & 3) == 0) get_mouse_mickeys(&mickeyx, &mickeyy);

	textprintf_ex(buffer, font, 16, 88, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mickey_x = %-7d", mickeyx);
	textprintf_ex(buffer, font, 16, 104, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mickey_y = %-7d", mickeyy);

	/* the mouse button state is stored in the variable mouse_b */
	if (mouse_b & 1)
		render_queue_submit_text(
				queue, UI_Z_ORDER, font, "left button pressed", 16, 128,
				makecol(0, 0, 0), makecol(255, 255, 255));
	else
		render_queue_submit_text(
				queue, UI_Z_ORDER, font, "left button not pressed", 16, 128,
				makecol(0, 0, 0), makecol(255, 255, 255));

	if (mouse_b & 2)
		render_queue_submit_text(
				queue, UI_Z_ORDER, font, "right button pressed", 16, 144,
				makecol(0, 0, 0), makecol(255, 255, 255));
	else
		render_queue_submit_text(
				queue, UI_Z_ORDER, font, "right button not pressed", 16, 144,
				makecol(0, 0, 0), makecol(255, 255, 255));

	if (mouse_b & 4)
		render_queue_submit_text(
				queue, UI_Z_ORDER, font, "middle button pressed", 16, 160,
				makecol(0, 0, 0), makecol(255, 255, 255));
	else
		render_queue_submit_text(
				queue, UI_Z_ORDER, font, "middle button not pressed", 16, 160,
				makecol(0, 0, 0), makecol(255, 255, 255));

	/* the wheel position is stored in the variable mouse_z */
	textprintf_ex(buffer, font, 16, 184, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mouse_z = %-5d mouse_w = %-5d", mouse_z, mouse_w);
}

int main(int argc, char *argv[]) {
	/* Init all systems */
	if (allegro_init() != ALLEGRO_INIT_OK) {
		printf("Error initializing Allegro.");
		return PROGRAM_ERROR;
	}
	if (install_keyboard() != ALLEGRO_INIT_OK) {
		printf("Error initializing keyboard.");
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

	/* Attach function to clos ebutton */
	LOCK_VARIABLE(closeButtonPressed);
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	game_snd_play_music(GAME_MUSIC_TITLE);

	if (game_mouse_init_cursors() != INITIALIZATION_OK) {
		printf("Error initializing mouse.");
		return PROGRAM_ERROR;
	}
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	set_mouse_sprite_focus(0, 0);

	game_snd_load_sounds();

	globalGameState.gameState = GAME_STATE_MAIN_MENU;
	render_queue_init(&renderQueue);

	BITMAP *screenBuffer = create_bitmap(GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT);
	memset(keyPrevious, FALSE, sizeof(keyPrevious));
	while (!closeButtonPressed && !key[KEY_ESC] && globalGameState.gameState != NUM_GAME_STATES) {
		memcpy(keyPrevious, (char *) key, sizeof(keyPrevious));
		poll_keyboard();
		// Execute game logic
		gameStateTable[globalGameState.gameState](&globalGameState);
		if (key[KEY_A] & !keyPrevious[KEY_A]) game_snd_play_sound(GAME_SOUND_SEA_WAVES);
		if (key[KEY_S] & !keyPrevious[KEY_S]) game_snd_play_sound(GAME_SOUND_CLICK);

		render_queue_submit_clear(&renderQueue, BACKGROUND_Z_ORDER, 0);
		render_queue_submit_sprite(&renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(), mouse_get_x() - mouse_x_focus, mouse_get_y() - mouse_y_focus, 0);
		vsync();

		// Render game
		printMouse(screenBuffer, &renderQueue);
		render_queue_execute(&renderQueue, screenBuffer);
		vsync();

		// Move buffer to screen
		acquire_screen();
		stretch_blit(screenBuffer, screen, 0, 0, screenBuffer->w, screenBuffer->h, 0, 0, screen->w, screen->h);
		release_screen();
	}

	snd_stop_music();
	snd_destroy_sounds();
	mouse_destroy_cursors();
	destroy_bitmap(screenBuffer);
	allegro_exit();
	return PROGRAM_OK;
}

END_OF_MAIN()

#include <stdio.h>
#include <allegro.h>
#include <jgmod.h>
#include "game/game.h"

#define PROGRAM_OK 0
#define PROGRAM_ERROR 1

#define INTERNAL_WIDTH 320
#define INTERNAL_HEIGHT 200
#ifdef DOS
#define EXTERNAL_WIDTH INTERNAL_WIDTH
#define EXTERNAL_HEIGHT INTERNAL_HEIGHT
#else
#define EXTERNAL_WIDTH INTERNAL_WIDTH * 3
#define EXTERNAL_HEIGHT INTERNAL_HEIGHT * 3
#endif

volatile int closeButtonPressed = FALSE;

void close_button_handler() {
	closeButtonPressed = TRUE;
}
END_OF_FUNCTION(close_button_handler)

int setVideoMode() {
#ifdef DOS
	if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, EXTERNAL_WIDTH, EXTERNAL_HEIGHT, 0, 0) != 0) {
#endif
		if (set_gfx_mode(GFX_SAFE, EXTERNAL_WIDTH, EXTERNAL_HEIGHT, 0, 0) != 0) return 1;
#ifdef DOS
	}
#endif
	return 0;
}

GameState globalGameState;

int mickeyx = 0;
int mickeyy = 0;
int c = 0;

#define ALLEGRO_INIT_OK 0

#define MOD_VOICES 8
#define GAME_VOICES 16

JGMOD *music;

void printMouse(BITMAP *buffer) {
	poll_mouse();

	textprintf_ex(buffer, font, 16, 48, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mouse_x = %-5d", mouse_x);
	textprintf_ex(buffer, font, 16, 64, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mouse_y = %-5d", mouse_y);

	/* or you can use this function to measure the speed of movement.
       * Note that we only call it every fourth time round the loop:
       * there's no need for that other than to slow the numbers down
       * a bit so that you will have time to read them...
       */
	c++;
	if ((c & 3) == 0) get_mouse_mickeys(&mickeyx, &mickeyy);

	textprintf_ex(buffer, font, 16, 88, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mickey_x = %-7d", mickeyx);
	textprintf_ex(buffer, font, 16, 104, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mickey_y = %-7d", mickeyy);

	/* the mouse button state is stored in the variable mouse_b */
	if (mouse_b & 1)
		textout_ex(buffer, font, "left button is pressed ", 16, 128,
				   makecol(0, 0, 0), makecol(255, 255, 255));
	else
		textout_ex(buffer, font, "left button not pressed", 16, 128,
				   makecol(0, 0, 0), makecol(255, 255, 255));

	if (mouse_b & 2)
		textout_ex(buffer, font, "right button is pressed ", 16, 144,
				   makecol(0, 0, 0), makecol(255, 255, 255));
	else
		textout_ex(buffer, font, "right button not pressed", 16, 144,
				   makecol(0, 0, 0), makecol(255, 255, 255));

	if (mouse_b & 4)
		textout_ex(buffer, font, "middle button is pressed ", 16, 160,
				   makecol(0, 0, 0), makecol(255, 255, 255));
	else
		textout_ex(buffer, font, "middle button not pressed", 16, 160,
				   makecol(0, 0, 0), makecol(255, 255, 255));

	/* the wheel position is stored in the variable mouse_z */
	textprintf_ex(buffer, font, 16, 184, makecol(0, 0, 0),
				  makecol(255, 255, 255), "mouse_z = %-5d mouse_w = %-5d", mouse_z, mouse_w);
}

int main(int argc, char *argv[]) {

	if (allegro_init() != ALLEGRO_INIT_OK) return PROGRAM_ERROR;
	if (install_keyboard() != ALLEGRO_INIT_OK) return PROGRAM_ERROR;

	set_color_depth(8);

	if (setVideoMode() != ALLEGRO_INIT_OK) {
		set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
		allegro_message("Unable to set any graphic mode\n%s\n", allegro_error);
		return PROGRAM_ERROR;
	}

	reserve_voices(GAME_VOICES, -1);
	if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) < ALLEGRO_INIT_OK) {
		printf("Error initializing sound card");
		return PROGRAM_ERROR;
	}

	if (install_mod(MOD_VOICES) < ALLEGRO_INIT_OK) {
		printf("Error setting digi voices");
		return PROGRAM_ERROR;
	}

	LOCK_VARIABLE(closeButtonPressed);
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

	music = load_mod("assets/menu.s3m");
	if (music == NULL) {
		printf("Error reading menu.s3m");
		return PROGRAM_ERROR;
	}
	play_mod(music, TRUE);

	BITMAP *buffer = create_bitmap(INTERNAL_WIDTH, INTERNAL_HEIGHT);

	if (install_mouse() < ALLEGRO_INIT_OK) {
		printf("Error installing mouse");
		return PROGRAM_ERROR;
	}

	BITMAP *mouseCursor = load_bitmap("assets/mouse/idle.pcx", NULL);
	show_mouse(NULL);
	set_mouse_sprite_focus(0, 0);

	globalGameState.gameState = GAME_STATE_MAIN_MENU;

	while (!closeButtonPressed && !key[KEY_ESC] && globalGameState.gameState != NUM_GAME_STATES) {
		clear_bitmap(buffer);
		gameStateTable[globalGameState.gameState](&globalGameState);
		printMouse(buffer);

		draw_sprite(
				buffer,
				mouseCursor,
				mouse_x - mouse_x_focus,
				mouse_y - mouse_y_focus);

		vsync();
		acquire_screen();
		stretch_blit(buffer, screen, 0, 0, buffer->w, buffer->h, 0, 0, screen->w, screen->h);
		release_screen();
	}

	stop_mod();
	destroy_mod(music);
	destroy_bitmap(buffer);
	destroy_bitmap(mouseCursor);
	allegro_exit();
	return PROGRAM_OK;
}

END_OF_MAIN()

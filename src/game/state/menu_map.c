#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

typedef enum {
	PAUSE_MENU_STATE_SELECT,
	PAUSE_MENU_STATE_SOUND,
	PAUSE_MENU_STATE_PLAYABILITY,
	PAUSE_MENU_STATE_CONFIRM,
	PAUSE_MENU_STATE_EXIT,
} PauseMenuStateEnum;

static BITMAP* background;
static PauseMenuStateEnum menuState;
static GuiElement* actualMenu;
static uint8_t actualMenuElements;
static MouseCursorStateEnum prevMouseCursorState;

static BITMAP* get_background_image() {
	return background;
}

static void exit_menu(GameContext* ) {
	menuState = PAUSE_MENU_STATE_EXIT;
}

static GuiElement backgroundImage = {
	.x = 0, .y = 0, .z = 0,
	.type = GUI_ELEMENT_IMAGE,
	.typed = { .image = { .getImage = get_background_image } }
};

static void select_menu_elements() {
	actualMenu = &backgroundImage;
	actualMenuElements = 1;
	/*switch (menuState) {

	}*/
}

extern BITMAP* get_screen_buffer();

GameStateEnum handle_init_menu_map(GameContext *context, RenderQueue *renderQueue) {
	BITMAP* screenBuffer = get_screen_buffer();
	background = create_bitmap(screenBuffer->w, screenBuffer->h);
	blit(screenBuffer, background, 0, 0, 0, 0, screenBuffer->w, screenBuffer->h);

	prevMouseCursorState = game_mouse_get_cursor_state();
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	menuState = PAUSE_MENU_STATE_SELECT;

	return GAME_STATE_MENU_MAP;
}

GameStateEnum handle_menu_map(GameContext *context, RenderQueue *renderQueue) {
	select_menu_elements();
	game_gui_handle(context, actualMenu, actualMenuElements);
	if(keyboard_is_key_pressed(KEY_F10)) exit_menu(context);

	if(menuState == PAUSE_MENU_STATE_EXIT) {
		game_mouse_set_cursor_state(prevMouseCursorState);
		destroy_bitmap(background);
		return GAME_STATE_PLAY_MAP;
	}
	else {
		game_gui_render_queue_submit(context, renderQueue, actualMenu, actualMenuElements);
		render_queue_submit_mouse(context, renderQueue);
		return GAME_STATE_MENU_MAP;
	}
}

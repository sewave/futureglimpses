#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define BUTTON_HEIGHT 18
#define BUTTON_Y_SEPARATION 20
#define BUTTON_MAIN_MENU_WIDTH 100
#define BUTTON_MAIN_MENU_X 110
#define BUTTON_MAIN_MENU_Y 60

typedef enum {
	PAUSE_MENU_STATE_SELECT,
	PAUSE_MENU_STATE_SOUND,
	PAUSE_MENU_STATE_GAMEPLAY,
	PAUSE_MENU_STATE_CONFIRM,
	PAUSE_MENU_STATE_EXIT,
	PAUSE_MENU_STATE_COUNT,
} PauseMenuStateEnum;

static BITMAP* background;
static PauseMenuStateEnum menuState;
static MouseCursorStateEnum prevMouseCursorState;
static uint8_t goMainMenu, goOS;

static void sound_menu(GameContext* context) {
	menuState = PAUSE_MENU_STATE_SOUND;
}

static void gameplay_menu(GameContext* context) {
	menuState = PAUSE_MENU_STATE_GAMEPLAY;
}

static void return_to_game(GameContext* context) {
	menuState = PAUSE_MENU_STATE_EXIT;
}

static void return_title(GameContext* context) {
	goMainMenu = TRUE;
}

static void exit_to_os(GameContext* context) {
	goOS = TRUE;
}

#define MAIN_MENU_ELEMENTS 7

static GuiElement mainMenu[MAIN_MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &background } }
	},
	{
		.x = 0, .y = 40, .z = 1,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = 320} }
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y, .z = 1,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_SOUND,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_S,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = sound_menu
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION, .z = 1,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_GAMEPLAY,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_G,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = gameplay_menu
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 2, .z = 1,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN_TO_GAME,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_R,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = return_to_game
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 3, .z = 1,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN_TITLE,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_T,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = return_title
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 4, .z = 1,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_EXIT_TO_OS,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_X,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = exit_to_os
			}
		}
	},
};

static GuiScreen guiScreens[PAUSE_MENU_STATE_COUNT] = {
	[PAUSE_MENU_STATE_SELECT] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_SOUND] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_GAMEPLAY] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_CONFIRM] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_EXIT] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
};

extern BITMAP* get_screen_buffer();

GameStateEnum handle_init_menu_map(GameContext *context, RenderQueue *renderQueue) {
	BITMAP* screenBuffer = get_screen_buffer();
	background = create_bitmap(screenBuffer->w, screenBuffer->h);
	blit(screenBuffer, background, 0, 0, 0, 0, screenBuffer->w, screenBuffer->h);

	prevMouseCursorState = game_mouse_get_cursor_state();
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	menuState = PAUSE_MENU_STATE_SELECT;
	goMainMenu = FALSE;
	goOS = FALSE;
	return GAME_STATE_MENU_MAP;
}

static void clean_up_menu() {
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	destroy_bitmap(background);
}

GameStateEnum handle_menu_map(GameContext *context, RenderQueue *renderQueue) {
	game_gui_handle(context, &guiScreens[menuState]);
	if(keyboard_is_key_pressed(KEY_F10)) return_to_game(context);
	if(goMainMenu) {
		clean_up_menu();
		return GAME_STATE_LOAD_MAP;
	}
	if(goOS) {
		clean_up_menu();
		return GAME_STATE_EXIT;
	}
	if(menuState == PAUSE_MENU_STATE_EXIT) {
		game_mouse_set_cursor_state(prevMouseCursorState);
		destroy_bitmap(background);
		return GAME_STATE_PLAY_MAP;
	}
	else {
		game_gui_render_queue_submit(context, renderQueue, &guiScreens[menuState]);
		render_queue_submit_mouse(context, renderQueue);
		return GAME_STATE_MENU_MAP;
	}
}

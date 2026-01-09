#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define MENU_BACK_X 80
#define MENU_BACK_Y 25
#define MENU_BACK_WIDTH 160
#define MENU_BACK_HEIGHT 160
#define MENU_TITLE_Y_OFFSET 10

#define BUTTON_HEIGHT 18
#define BUTTON_Y_SEPARATION 20
#define BUTTON_MAIN_MENU_WIDTH 100
#define BUTTON_MAIN_MENU_X 110
#define BUTTON_MAIN_MENU_Y 60

#define BUTTON_RETURN_WIDTH 100
#define BUTTON_RETURN_X MENU_BACK_X + MENU_BACK_WIDTH / 2 - BUTTON_RETURN_WIDTH / 2
#define BUTTON_RETURN_Y 140

typedef enum {
	PAUSE_MENU_STATE_SELECT,
	PAUSE_MENU_STATE_SOUND,
	PAUSE_MENU_STATE_GAMEPLAY,
	PAUSE_MENU_STATE_CONFIRM,
	PAUSE_MENU_STATE_EXIT,
	PAUSE_MENU_STATE_COUNT,
} PauseMenuStateEnum;

static BITMAP* background;
static BITMAP* menuBack;
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

static void main_menu(GameContext* context) {
	menuState = PAUSE_MENU_STATE_SELECT;
}

static uint8_t get_gameplay_life_bars(const GameContext *context) {
	return context->config.lifeBar;
}

static void set_gameplay_life_bars(GameContext *context, uint8_t value) {
	context->config.lifeBar = (LifeBarEnum) value;
}

#define MAIN_MENU_ELEMENTS 8

static GuiElement mainMenu[MAIN_MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &background } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y, .z = 1,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &menuBack } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 2,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y, .z = 10,
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
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION, .z = 10,
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
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 2, .z = 10,
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
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 3, .z = 10,
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
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 4, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_EXIT_TO_OS,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_O,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = exit_to_os
			}
		}
	},
};

#define GAMEPLAY_MENU_ELEMENTS 5

static GuiElement gameplayMenu[GAMEPLAY_MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &background } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y, .z = 1,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &menuBack } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 2,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_GAMEPLAY_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_RETURN_X, .y = BUTTON_RETURN_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN_MAIN_MENU,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_R,
		.typed = {
			.button = {
				.size = { .width = BUTTON_RETURN_WIDTH, .height = BUTTON_HEIGHT },
				.action = main_menu
			}
		}
	},
	{
		.x = BUTTON_RETURN_X, .y = MENU_BACK_Y + 40, .z = 10,
		.type = GUI_ELEMENT_OPTION,
		.textId = GAME_TEXT_ID_MENU_GAMEPLAY_LIFE_BARS,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.option = {
				.optionValuesNumber = 3,
				.optionValues = (GuiOptionValue[]) {
					{ .value = LIFE_BAR_ALWAYS, .textId = GAME_TEXT_ID_MENU_GAMEPLAY_LIFE_BARS_ALWAYS, .hotkey = KEY_A,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX },
					{ .value = LIFE_BAR_DAMAGED, .textId = GAME_TEXT_ID_MENU_GAMEPLAY_LIFE_BARS_ONLY_DAMAGED, .hotkey = KEY_D,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX },
					{ .value = LIFE_BAR_NEVER, .textId = GAME_TEXT_ID_MENU_GAMEPLAY_LIFE_BARS_NEVER, .hotkey = KEY_N,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX },
				},
				.getValue = get_gameplay_life_bars,
				.setValue = set_gameplay_life_bars
			}
		}
	},
};


#define SOUND_MENU_ELEMENTS 4

static GuiElement soundMenu[SOUND_MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &background } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y, .z = 1,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &menuBack } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 2,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_SOUND_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_RETURN_X, .y = BUTTON_RETURN_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN_MAIN_MENU,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_R,
		.typed = {
			.button = {
				.size = { .width = BUTTON_RETURN_WIDTH, .height = BUTTON_HEIGHT },
				.action = main_menu
			}
		}
	},
};

static GuiScreen guiScreens[PAUSE_MENU_STATE_COUNT] = {
	[PAUSE_MENU_STATE_SELECT] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_SOUND] = { .elements = soundMenu, .elementsCount = SOUND_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_GAMEPLAY] = { .elements = gameplayMenu, .elementsCount = GAMEPLAY_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_CONFIRM] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_EXIT] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
};

extern BITMAP* get_screen_buffer();

GameStateEnum handle_init_menu_map(GameContext *context, RenderQueue *renderQueue) {
	BITMAP* screenBuffer = get_screen_buffer();
	background = create_bitmap(screenBuffer->w, screenBuffer->h);
	blit(screenBuffer, background, 0, 0, 0, 0, screenBuffer->w, screenBuffer->h);
	menuBack = game_gfx_get_menu_back();

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

#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define TITLE_BACKGROUND_PATH "assets/ui/title.pcx"

typedef enum {
	TITLE_MENU_INITIAL,
	TITLE_MENU_START_GAME,
    TITLE_MENU_OPTIONS,
	TITLE_MENU_AUDIO,
	TITLE_MENU_GAMEPLAY,
	TITLE_MENU_EXIT,
    TITLE_MENU_CONFIRM_EXIT,
	TITLE_MENU_STATE_COUNT
} TitleMenuStateEnum;

static TitleMenuStateEnum titleMenuState;
static BITMAP *titleBackground;

static void start_game(GameContext *context) {
	titleMenuState = TITLE_MENU_START_GAME;
}

static void exit_game(GameContext *context) {
	titleMenuState = TITLE_MENU_EXIT;
}

static void audio_settings(GameContext *context) {
	titleMenuState = TITLE_MENU_AUDIO;
}

static void gameplay_settings(GameContext *context) {
	titleMenuState = TITLE_MENU_GAMEPLAY;
}

static void options_settings(GameContext *context) {
	titleMenuState = TITLE_MENU_OPTIONS;
}

static void init_settings(GameContext *context) {
	titleMenuState = TITLE_MENU_INITIAL;
}

static void confirm_os(GameContext* context) {
	titleMenuState = TITLE_MENU_CONFIRM_EXIT;
}

#define TITLE_MENU_START_Y 135
#define TITLE_MENU_SPACING 20
#define TITLE_MENU_BUTTON_WIDTH 120
#define TITLE_MENU_BUTTON_HEIGHT 18
#define TITLE_MENU_BUTTON_X 100

#define TITLE_MENU_ELEMENTS 4

static GuiElement titleMenu[TITLE_MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y, .z = UI_Z_ORDER + 900,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_START_GAME,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .hotkey = KEY_S,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = start_game
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING, .z = UI_Z_ORDER + 901,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_OPTIONS,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .hotkey = KEY_P,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = options_settings
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 2, .z = UI_Z_ORDER + 903,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_EXIT_TO_OS,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .hotkey = KEY_O,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = confirm_os
            }
        }
    },
};

#define TITLE_MENU_OPTIONS_ELEMENTS 4

static GuiElement optionsMenu[TITLE_MENU_OPTIONS_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y, .z = UI_Z_ORDER + 901,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_SOUND,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .hotkey = KEY_S,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = audio_settings
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING, .z = UI_Z_ORDER + 902,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_GAMEPLAY,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .hotkey = KEY_G,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = gameplay_settings
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 2, .z = UI_Z_ORDER + 903,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_RETURN,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .hotkey = KEY_R,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = init_settings
            }
        }
    },
};

#define TITLE_MENU_CONFIRM_EXIT_ELEMENTS 4

static GuiElement confirmOSMenu[TITLE_MENU_CONFIRM_EXIT_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
	{
		.x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y, .z = UI_Z_ORDER + 900,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_CONFIRM_TITLE,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = TITLE_MENU_BUTTON_X + TITLE_MENU_BUTTON_WIDTH } }
	},
	{
		.x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING, .z = UI_Z_ORDER + 901,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_YES,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_S,
		.typed = {
			.button = {
				.size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
				.action = exit_game
			}
		}
	},
	{
		.x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 2, .z = UI_Z_ORDER + 902,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_NO,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_N,
		.typed = {
			.button = {
				.size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
				.action = init_settings
			}
		}
	},
};

static GuiScreen guiScreens[TITLE_MENU_STATE_COUNT] = {
		[TITLE_MENU_INITIAL] = {.elements = titleMenu, .elementsCount = TITLE_MENU_ELEMENTS},
		// We won't print anything in here
		[TITLE_MENU_START_GAME] = {.elements = NULL, .elementsCount = 0},
        [TITLE_MENU_OPTIONS] = {.elements = optionsMenu, .elementsCount = TITLE_MENU_OPTIONS_ELEMENTS},
		[TITLE_MENU_AUDIO] = {.elements = optionsMenu, .elementsCount = TITLE_MENU_OPTIONS_ELEMENTS},
		[TITLE_MENU_GAMEPLAY] = {.elements = optionsMenu, .elementsCount = TITLE_MENU_OPTIONS_ELEMENTS},
        [TITLE_MENU_CONFIRM_EXIT] = {.elements = confirmOSMenu, .elementsCount = TITLE_MENU_CONFIRM_EXIT_ELEMENTS},
		// We won't print anything in here
		[TITLE_MENU_EXIT] = {.elements = NULL, .elementsCount = 0}
};

GameStateEnum handle_init_title(GameContext *context, RenderQueue *renderQueue) {
	titleMenuState = TITLE_MENU_INITIAL;
	titleBackground = load_bitmap(TITLE_BACKGROUND_PATH, NULL);
	if (!titleBackground) {
		fprintf(stderr, "Error loading title background bitmap: %s\n", TITLE_BACKGROUND_PATH);
		exit(EXIT_FAILURE);
	}
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	return GAME_STATE_TITLE;
}

GameStateEnum handle_title(GameContext *context, RenderQueue *renderQueue) {
	GuiScreen *guiScreen = &guiScreens[titleMenuState];
	game_gui_handle(context, guiScreen);

	if (titleMenuState == TITLE_MENU_START_GAME) {
		// TODO go to stage select when implemented
		destroy_bitmap(titleBackground);
		return GAME_STATE_LOAD_MAP;
	} else if (titleMenuState == TITLE_MENU_EXIT) {
		destroy_bitmap(titleBackground);
		return GAME_STATE_EXIT;
	}

	game_gui_render_queue_submit(context, renderQueue, guiScreen);
	render_queue_submit_mouse(context, renderQueue);
	return GAME_STATE_TITLE;
}

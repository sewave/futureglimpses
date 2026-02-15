#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include <allegro.h>

#define BUTTON_Y_SEPARATION 20
#define BUTTON_MAIN_MENU_WIDTH 100
#define BUTTON_MAIN_MENU_X 110
#define BUTTON_MAIN_MENU_Y 50

#define BUTTON_RETURN_WIDTH 100
#define BUTTON_RETURN_X MENU_BACK_X + MENU_BACK_WIDTH / 2 - BUTTON_RETURN_WIDTH / 2
#define BUTTON_RETURN_Y 150

#define BUTTON_CONFIRM_WIDTH 60
#define BUTTON_CONFIRM_X_YES MENU_BACK_X + 10
#define BUTTON_CONFIRM_X_NO BUTTON_CONFIRM_X_YES + BUTTON_CONFIRM_WIDTH + 20
#define BUTTON_CONFIRM_Y 140

typedef enum {
	PAUSE_MENU_STATE_SELECT,
	PAUSE_MENU_STATE_MAP,
	PAUSE_MENU_STATE_SOUND,
	PAUSE_MENU_STATE_GAMEPLAY,
	PAUSE_MENU_STATE_CONFIRM_TITLE,
	PAUSE_MENU_STATE_CONFIRM_OS,
	PAUSE_MENU_STATE_EXIT,
	PAUSE_MENU_STATE_EXIT_OS,
	PAUSE_MENU_STATE_EXIT_TITLE,
	PAUSE_MENU_STATE_COUNT,
} PauseMenuStateEnum;

static BITMAP* background;
static BITMAP* menuBack;
static PauseMenuStateEnum menuState;
static MouseCursorStateEnum prevMouseCursorState;

static void sound_menu(GameContext* context) {
	menuState = PAUSE_MENU_STATE_SOUND;
}

static void gameplay_menu(GameContext* context) {
	menuState = PAUSE_MENU_STATE_GAMEPLAY;
}

static void return_to_game(GameContext* context) {
	menuState = PAUSE_MENU_STATE_EXIT;
}

static void map_menu(GameContext* context) {
	menuState = PAUSE_MENU_STATE_MAP;
}

static void return_title(GameContext* context) {
	menuState = PAUSE_MENU_STATE_EXIT_TITLE;
}

static void exit_to_os(GameContext* context) {
	menuState = PAUSE_MENU_STATE_EXIT_OS;
}

static void main_menu(GameContext* context) {
	menuState = PAUSE_MENU_STATE_SELECT;
}

static void confirm_title(GameContext* context) {
	menuState = PAUSE_MENU_STATE_CONFIRM_TITLE;
}

static void confirm_os(GameContext* context) {
	menuState = PAUSE_MENU_STATE_CONFIRM_OS;
}

static char* get_map_title(const GameContext* context) {
	return context->map.title;
}

static char* get_map_description(const GameContext* context) {
	return context->map.description;
}

#define MAIN_MENU_ELEMENTS 9

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
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_MAP,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_M,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = map_menu
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_SOUND,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_S,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = sound_menu
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 2, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_GAMEPLAY,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_G,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = gameplay_menu
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 3, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN_TO_GAME,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_R,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = return_to_game
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 4, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN_TITLE,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_T,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = confirm_title
			}
		}
	},
	{
		.x = BUTTON_MAIN_MENU_X, .y = BUTTON_MAIN_MENU_Y + BUTTON_Y_SEPARATION * 5, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_EXIT_TO_OS,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_O,
		.typed = {
			.button = {
				.size = { .width = BUTTON_MAIN_MENU_WIDTH, .height = BUTTON_HEIGHT },
				.action = confirm_os
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
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_GAMEPLAY_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_RETURN_X, .y = BUTTON_RETURN_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
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
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = {
			.option = {
				.optionValuesNumber = 3,
				.optionValues = (GuiOptionValue[]) {
					{ .value = LIFE_BAR_ALWAYS, .textId = GAME_TEXT_ID_MENU_GAMEPLAY_LIFE_BARS_ALWAYS, .hotkey = KEY_A,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX, .shadowTextColor = PAL_COLOR_BLACK },
					{ .value = LIFE_BAR_DAMAGED, .textId = GAME_TEXT_ID_MENU_GAMEPLAY_LIFE_BARS_ONLY_DAMAGED, .hotkey = KEY_D,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX, .shadowTextColor = PAL_COLOR_BLACK },
					{ .value = LIFE_BAR_NEVER, .textId = GAME_TEXT_ID_MENU_GAMEPLAY_LIFE_BARS_NEVER, .hotkey = KEY_N,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX, .shadowTextColor = PAL_COLOR_BLACK },
				},
				.getValue = game_config_get_gameplay_life_bars,
				.setValue = game_config_set_gameplay_life_bars
			}
		}
	},
};

#define SOUND_MENU_ELEMENTS 6

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
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_SOUND_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_RETURN_X, .y = BUTTON_RETURN_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_R,
		.typed = {
			.button = {
				.size = { .width = BUTTON_RETURN_WIDTH, .height = BUTTON_HEIGHT },
				.action = main_menu
			}
		}
	},
	{
		.x = MENU_BACK_X + 10, .y = MENU_BACK_Y + 30, .z = 10,
		.type = GUI_ELEMENT_BAR,
		.textId = GAME_TEXT_ID_MENU_SOUND_MUSIC_VOLUME,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = {
			.bar = {
				.getMaxValue = game_config_get_audio_max_volume,
				.getMinValue = game_config_get_audio_min_volume,
				.getValue = game_config_get_audio_music_volume,
				.setValue = game_config_set_audio_music_volume,
				.valueInc = 16,
			}
		}
	},
	{
		.x = MENU_BACK_X + 10, .y = MENU_BACK_Y + 70, .z = 10,
		.type = GUI_ELEMENT_BAR,
		.textId = GAME_TEXT_ID_MENU_SOUND_EFFECTS_VOLUME,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = {
			.bar = {
				.getMaxValue = game_config_get_audio_max_volume,
				.getMinValue = game_config_get_audio_min_volume,
				.getValue = game_config_get_audio_sfx_volume,
				.setValue = game_config_set_audio_sfx_volume,
				.valueInc = 16,
			}
		}
	},
};

#define CONFIRM_TITLE_MENU_ELEMENTS 5

static GuiElement confirmTitleMenu[CONFIRM_TITLE_MENU_ELEMENTS] = {
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
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_CONFIRM_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_CONFIRM_X_YES, .y = BUTTON_CONFIRM_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_YES,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_S,
		.typed = {
			.button = {
				.size = { .width = BUTTON_CONFIRM_WIDTH, .height = BUTTON_HEIGHT },
				.action = return_title
			}
		}
	},
	{
		.x = BUTTON_CONFIRM_X_NO, .y = BUTTON_CONFIRM_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_NO,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_N,
		.typed = {
			.button = {
				.size = { .width = BUTTON_CONFIRM_WIDTH, .height = BUTTON_HEIGHT },
				.action = main_menu
			}
		}
	},
};

#define CONFIRM_OS_MENU_ELEMENTS 5

static GuiElement confirmOSMenu[CONFIRM_OS_MENU_ELEMENTS] = {
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
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_CONFIRM_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_CONFIRM_X_YES, .y = BUTTON_CONFIRM_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_YES,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_S,
		.typed = {
			.button = {
				.size = { .width = BUTTON_CONFIRM_WIDTH, .height = BUTTON_HEIGHT },
				.action = exit_to_os
			}
		}
	},
	{
		.x = BUTTON_CONFIRM_X_NO, .y = BUTTON_CONFIRM_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_NO,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_N,
		.typed = {
			.button = {
				.size = { .width = BUTTON_CONFIRM_WIDTH, .height = BUTTON_HEIGHT },
				.action = main_menu
			}
		}
	},
};

#define MAP_MENU_ELEMENTS 8

static GuiElement mapMenu[MAP_MENU_ELEMENTS] = {
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
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_MAP_MENU_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = BUTTON_RETURN_X, .y = BUTTON_RETURN_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_R,
		.typed = {
			.button = {
				.size = { .width = BUTTON_RETURN_WIDTH, .height = BUTTON_HEIGHT },
				.action = main_menu
			}
		}
	},
	{
		.x = MENU_BACK_X + 5, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET + 10, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_MAP_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { } }
	},
	{
		.x = MENU_BACK_X + 5, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET + 30, .z = 5,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_MENU_MAP_DESCRIPTION,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { } }
	},
	{
		.x = MENU_BACK_X + 5, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET + 20, .z = 5,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .customText = { .text = get_map_title } }
	},
	{
		.x = MENU_BACK_X + 5, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET + 40, .z = 5,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .customText = {
					.text = get_map_description,
					.maxWidth = MENU_BACK_WIDTH - 10,
					.maxHeight = 80
					}
				}
	},
};

static GuiScreen guiScreens[PAUSE_MENU_STATE_COUNT] = {
	[PAUSE_MENU_STATE_SELECT] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_MAP] = { .elements = mapMenu, .elementsCount = MAP_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_SOUND] = { .elements = soundMenu, .elementsCount = SOUND_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_GAMEPLAY] = { .elements = gameplayMenu, .elementsCount = GAMEPLAY_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_CONFIRM_TITLE] = { .elements = confirmTitleMenu, .elementsCount = CONFIRM_TITLE_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_CONFIRM_OS] = { .elements = confirmOSMenu, .elementsCount = CONFIRM_OS_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_EXIT] = { .elements = mainMenu, .elementsCount = MAIN_MENU_ELEMENTS },
	[PAUSE_MENU_STATE_EXIT_OS] = { .elements = NULL, .elementsCount = 0 },
	[PAUSE_MENU_STATE_EXIT_TITLE] = { .elements = NULL, .elementsCount = 0 },
};

static void clean_up_menu() {
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	destroy_bitmap(background);
}

void handle_menu_map_render(GameContext *context, RenderQueue *renderQueue) {
	game_gui_render_queue_submit(context, renderQueue, &guiScreens[menuState]);
	render_queue_submit_mouse(context, renderQueue);
}

void handle_menu_map_init(GameContext *context) {
	BITMAP* screenBuffer = context->screenBuffer;
	background = create_bitmap(screenBuffer->w, screenBuffer->h);
	blit(screenBuffer, background, 0, 0, 0, 0, screenBuffer->w, screenBuffer->h);
	menuBack = game_gfx_get_menu_back();

	prevMouseCursorState = game_mouse_get_cursor_state();
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	menuState = PAUSE_MENU_STATE_SELECT;
}

GameStateEnum handle_menu_map_update(GameContext *context) {
	game_gui_handle(context, &guiScreens[menuState]);
	GameStateEnum nextState;
	switch(menuState) {
		case PAUSE_MENU_STATE_EXIT_TITLE: {
			clean_up_menu();
			video_fade_out_init(DEFAULT_FADE_SPEED);
			nextState = GAME_STATE_TITLE;
			break;
		}
		case PAUSE_MENU_STATE_EXIT_OS: {
			clean_up_menu();
			video_fade_out_init(DEFAULT_FADE_SPEED);
			nextState = GAME_STATE_EXIT;
			break;
		}
		case PAUSE_MENU_STATE_EXIT: {
			game_mouse_set_cursor_state(prevMouseCursorState);
			destroy_bitmap(background);
			video_fade_in_skip_next();
			nextState = GAME_STATE_PLAY_MAP;
			break;
		}
		default: {
			nextState = GAME_STATE_MENU_MAP;
			break;
		}
	}
	return nextState;
}

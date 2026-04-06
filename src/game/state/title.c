#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include <allegro.h>

#define TITLE_BACKGROUND_PATH "assets/gfx/ui/back/title.pcx"
#define TITLE_TEXT_ES_PATH "assets/gfx/ui/t_txt_es.pcx"
#define TITLE_TEXT_EN_PATH "assets/gfx/ui/t_txt_en.pcx"

typedef enum {
	TITLE_MENU_INITIAL,
	TITLE_MENU_START_GAME,
    TITLE_MENU_OPTIONS,
	TITLE_MENU_AUDIO,
	TITLE_MENU_GAMEPLAY,
	TITLE_MENU_EXIT,
    TITLE_MENU_CONFIRM_EXIT,
	TITLE_MENU_CREDITS,
	TITLE_MENU_STATE_COUNT
} TitleMenuStateEnum;

static TitleMenuStateEnum titleMenuState;
static BITMAP *titleBackground;
static BITMAP *titleText;

static void load_title_text_image(GameContext *context) {
	char* titleTextPath;
	switch (context->config.language) {
		case LANGUAGE_SPANISH:
			titleTextPath = TITLE_TEXT_ES_PATH;
			break;
		case LANGUAGE_ENGLISH:
		default:
			titleTextPath = TITLE_TEXT_EN_PATH;
			break;
	}
	titleText = load_bitmap(titleTextPath, NULL);
	if (!titleText) {
		fprintf(stderr, "Error loading title text bitmap: %s\n", titleTextPath);
		exit(EXIT_FAILURE);
	}
}

static void set_reload_language(GameContext *context, uint8_t value) {
	game_config_set_language(context, value);
	load_title_text_image(context);
}

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

static void credits(GameContext* context) {
	titleMenuState = TITLE_MENU_CREDITS;
}

#define TITLE_MENU_START_Y 120
#define TITLE_MENU_SPACING 20
#define TITLE_MENU_BUTTON_WIDTH 120
#define TITLE_MENU_BUTTON_HEIGHT 18
#define TITLE_MENU_BUTTON_X 100
#define TITLE_MENU_Y 130
#define TITLE_MENU_X 80
#define TITLE_MENU_WIDTH 170
#define TITLE_MENU_HEIGHT 115

#define TITLE_MENU_ELEMENTS 6

static GuiElement titleMenu[TITLE_MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
	{
		.x = 20, .y = 10, .z = 1,
		.type = GUI_ELEMENT_SPRITE,
		.typed = { .image = { .bitmap = &titleText } }
	},
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_START_GAME,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_A,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = start_game
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_OPTIONS,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_P,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = options_settings
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 2 , .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_CREDITS_BUTTON,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
        .shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_C,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = credits
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 3, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_EXIT_TO_OS,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_O,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = confirm_os
            }
        }
    },
};

#define TITLE_MENU_OPTIONS_ELEMENTS 5

static GuiElement optionsMenu[TITLE_MENU_OPTIONS_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
	{
		.x = 20, .y = 10, .z = 1,
		.type = GUI_ELEMENT_SPRITE,
		.typed = { .image = { .bitmap = &titleText } }
	},
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_SOUND,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_S,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = audio_settings
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_GAMEPLAY,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_G,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = gameplay_settings
            }
        }
    },
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 3, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_RETURN,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_R,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = init_settings
            }
        }
    },
};

#define TITLE_MENU_CONFIRM_EXIT_ELEMENTS 5

static GuiElement confirmOSMenu[TITLE_MENU_CONFIRM_EXIT_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
	{
		.x = 20, .y = 10, .z = 1,
		.type = GUI_ELEMENT_SPRITE,
		.typed = { .image = { .bitmap = &titleText } }
	},
	{
		.x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y, .z = 1,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_CONFIRM_TITLE,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = { .text = { .maxX = TITLE_MENU_BUTTON_X + TITLE_MENU_BUTTON_WIDTH } }
	},
	{
		.x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING, .z = 1,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_YES,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_S,
		.typed = {
			.button = {
				.size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
				.action = exit_game
			}
		}
	},
	{
		.x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 2, .z = 1,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_NO,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_N,
		.typed = {
			.button = {
				.size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
				.action = init_settings
			}
		}
	},
};

#define TITLE_MENU_SOUND_ELEMENTS 5

static GuiElement soundMenu[TITLE_MENU_SOUND_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
	{
		.x = 20, .y = 10, .z = 1,
		.type = GUI_ELEMENT_SPRITE,
		.typed = { .image = { .bitmap = &titleText } }
	},
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 3, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_RETURN,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_R,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = options_settings
            }
        }
    },
	{
		.x = TITLE_MENU_X + 10, .y = TITLE_MENU_Y - 15, .z = 2,
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
		.x = TITLE_MENU_X + 10, .y = TITLE_MENU_Y + 10, .z = 2,
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

#define GAMEPLAY_MENU_ELEMENTS 5

static GuiElement gameplayMenu[GAMEPLAY_MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
	{
		.x = 20, .y = 10, .z = 1,
		.type = GUI_ELEMENT_SPRITE,
		.typed = { .image = { .bitmap = &titleText } }
	},
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 3, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_RETURN,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_R,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = options_settings
            }
        }
    },
	{
		.x = TITLE_MENU_X, .y = TITLE_MENU_Y - 15, .z = 10,
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
	{
		.x = TITLE_MENU_X + 100, .y = TITLE_MENU_Y - 15, .z = 11,
		.type = GUI_ELEMENT_OPTION,
		.textId = GAME_TEXT_ID_MENU_LANGUAGE,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = {
			.option = {
				.optionValuesNumber = 2,
				.optionValues = (GuiOptionValue[]) {
					{ .value = LANGUAGE_SPANISH, .textId = GAME_TEXT_ID_MENU_LANGUAGE_SPANISH, .hotkey = KEY_E,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX, .shadowTextColor = PAL_COLOR_BLACK },
					{ .value = LANGUAGE_ENGLISH, .textId = GAME_TEXT_ID_MENU_LANGUAGE_ENGLISH, .hotkey = KEY_S,
						.textColor = PAL_COLOR_WHITE, .textBackground = TRANSPARENT_INDEX, .shadowTextColor = PAL_COLOR_BLACK },
				},
				.getValue = game_config_get_language,
				.setValue = set_reload_language
			}
		}
	},
};

#define TITLE_MENU_CREDITS_ELEMENTS 5
#define CREDITS_Y 10
#define CREDITS_X 10
#define CREDITS_Y_OFFSET 12

static GuiElement creditsMenu[TITLE_MENU_CREDITS_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &titleBackground } }
	},
    {
        .x = TITLE_MENU_BUTTON_X, .y = TITLE_MENU_START_Y + TITLE_MENU_SPACING * 3, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_RETURN,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_R,
        .typed = {
            .button = {
                .size = { .width = TITLE_MENU_BUTTON_WIDTH, .height = TITLE_MENU_BUTTON_HEIGHT },
                .action = init_settings
            }
        }
    },
    {
        .x = CREDITS_X, .y = CREDITS_Y - CREDITS_Y_OFFSET / 2 + 2, .z = 1,
        .type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_CREDITS_TITLE,
        .textColor = PAL_COLOR_YELLOW,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .typed = { .text = { .maxX = GAME_INTERNAL_WIDTH - CREDITS_X, } }
    },
    {
        .x = CREDITS_X, .y = CREDITS_Y + CREDITS_Y_OFFSET, .z = 1,
        .type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_CREDITS_LINE_1,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = { .text = { .maxWidth = GAME_INTERNAL_WIDTH - CREDITS_X, .maxHeight = CREDITS_Y_OFFSET * 10 } }
    },
    {
        .x = CREDITS_X, .y = CREDITS_Y + CREDITS_Y_OFFSET * 11, .z = 1,
        .type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_CREDITS_LINE_2,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
    },
};

static GuiScreen guiScreens[TITLE_MENU_STATE_COUNT] = {
		[TITLE_MENU_INITIAL] = {.elements = titleMenu, .elementsCount = TITLE_MENU_ELEMENTS},
		// We won't print anything in here
		[TITLE_MENU_START_GAME] = {.elements = NULL, .elementsCount = 0},
        [TITLE_MENU_OPTIONS] = {.elements = optionsMenu, .elementsCount = TITLE_MENU_OPTIONS_ELEMENTS},
		[TITLE_MENU_AUDIO] = {.elements = soundMenu, .elementsCount = TITLE_MENU_SOUND_ELEMENTS},
		[TITLE_MENU_GAMEPLAY] = {.elements = gameplayMenu, .elementsCount = GAMEPLAY_MENU_ELEMENTS},
        [TITLE_MENU_CONFIRM_EXIT] = {.elements = confirmOSMenu, .elementsCount = TITLE_MENU_CONFIRM_EXIT_ELEMENTS},
		[TITLE_MENU_CREDITS] = {.elements = creditsMenu, .elementsCount = TITLE_MENU_CREDITS_ELEMENTS},
		// We won't print anything in here
		[TITLE_MENU_EXIT] = {.elements = NULL, .elementsCount = 0}
};

void handle_title_render(GameContext *context, RenderQueue *renderQueue) {
	game_gui_render_queue_submit(context, renderQueue, &guiScreens[titleMenuState]);
	render_queue_submit_mouse(context, renderQueue);
}

void handle_title_init(GameContext *context) {
	titleMenuState = TITLE_MENU_INITIAL;
	titleBackground = load_bitmap(TITLE_BACKGROUND_PATH, NULL);
	if (!titleBackground) {
		fprintf(stderr, "Error loading title background bitmap: %s\n", TITLE_BACKGROUND_PATH);
		exit(EXIT_FAILURE);
	}
	load_title_text_image(context);
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	game_snd_play_music(GAME_MUSIC_MENUS);
	video_fade_in_init(DEFAULT_FADE_SPEED, context->mainPalette);
}

GameStateEnum handle_title_update(GameContext *context) {
	GuiScreen *guiScreen = &guiScreens[titleMenuState];
	game_gui_handle(context, guiScreen);
	if (titleMenuState == TITLE_MENU_START_GAME) {
		destroy_bitmap(titleBackground);
		destroy_bitmap(titleText);
		video_fade_out_init(DEFAULT_FADE_SPEED);
		return GAME_STATE_SCENARIO_SELECT;
	} else if (titleMenuState == TITLE_MENU_EXIT) {
		destroy_bitmap(titleBackground);
		destroy_bitmap(titleText);
		video_fade_out_init(DEFAULT_FADE_SPEED);
		return GAME_STATE_EXIT;
	}
	return GAME_STATE_TITLE;
}

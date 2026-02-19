#include "game/state/results.h"
#include "game/game_lib.h"
#include <allegro.h>

#define RESULTS_BACKGROUND_PATH "assets/ui/results.pcx"
#define CONTROLLER_STATS 8
#define CONTROLLER_STAT_MAX_LENGTH 16
#define BASE_TEN_NUMER 10
#define UNITS_TRAINED_INDEX 0
#define ENEMIES_KILLED_INDEX 1
#define BUILDINGS_CONSTRUCTED_INDEX 2
#define BUILDINGS_DESTROYED_INDEX 3
#define GOLD_GATHERED_INDEX 4
#define WOOD_GATHERED_INDEX 5
#define GOLD_SPENT_INDEX 6
#define WOOD_SPENT_INDEX 7

static BITMAP *resultsBackground;

static char resultNumbers[UNIT_CONTROLLERS_COUNT][CONTROLLER_STATS][CONTROLLER_STAT_MAX_LENGTH];

static uint8_t goTitle;

static void return_to_title(GameContext *context) {
    goTitle = TRUE;
}

static char* get_result_text(const GameContext *context) {
    if(context->gameResult == GAME_RESULT_VICTORY) {
        return (char *) text_get_by_id(GAME_TEXT_ID_RESULT_VICTORY);
    } else if(context->gameResult == GAME_RESULT_DEFEAT) {
        return (char *) text_get_by_id(GAME_TEXT_ID_RESULT_DEFEAT);
    }
    return "";
}

#define MENU_ELEMENTS 3
#define MENU_BUTTON_WIDTH 120
#define MENU_BUTTON_HEIGHT 18
#define MENU_BUTTON_X 100
#define MENU_BUTTON_Y 170

static GuiElement menu[MENU_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &resultsBackground } }
	},
    {
        .x = MENU_BUTTON_X, .y = MENU_BUTTON_Y, .z = 1,
        .type = GUI_ELEMENT_BUTTON,
        .textId = GAME_TEXT_ID_MENU_RETURN_TITLE,
        .textColor = PAL_COLOR_WHITE,
        .textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
        .hotkey = KEY_R,
        .typed = {
            .button = {
                .size = { .width = MENU_BUTTON_WIDTH, .height = MENU_BUTTON_HEIGHT },
                .action = return_to_title
            }
        }
    },
	{
		.x = 0, .y = 20, .z = 5,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .customText = { .text = get_result_text, .maxX = 320 } }
	},
};

static GuiScreen guiScreen = {.elements = menu, .elementsCount = MENU_ELEMENTS};

void handle_results_init(GameContext *context) {
    resultsBackground = load_bitmap(RESULTS_BACKGROUND_PATH, NULL);
    if (!resultsBackground) {
        TRACE("Error loading results background bitmap: %s\n", RESULTS_BACKGROUND_PATH);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < UNIT_CONTROLLERS_COUNT; i++) {
        itoa(context->stats[i].unitsTrained, resultNumbers[i][UNITS_TRAINED_INDEX], BASE_TEN_NUMER);
        itoa(context->stats[i].enemiesKilled, resultNumbers[i][ENEMIES_KILLED_INDEX], BASE_TEN_NUMER);
        itoa(context->stats[i].buildingsConstructed, resultNumbers[i][BUILDINGS_CONSTRUCTED_INDEX], BASE_TEN_NUMER);
        itoa(context->stats[i].buildingsDestroyed, resultNumbers[i][BUILDINGS_DESTROYED_INDEX], BASE_TEN_NUMER);
        itoa(context->stats[i].resourcesGathered[RESOURCE_TYPE_GOLD], resultNumbers[i][GOLD_GATHERED_INDEX], BASE_TEN_NUMER);
        itoa(context->stats[i].resourcesGathered[RESOURCE_TYPE_WOOD], resultNumbers[i][WOOD_GATHERED_INDEX], BASE_TEN_NUMER);
        itoa(context->stats[i].resourcesSpent[RESOURCE_TYPE_GOLD], resultNumbers[i][GOLD_SPENT_INDEX], BASE_TEN_NUMER);
        itoa(context->stats[i].resourcesSpent[RESOURCE_TYPE_WOOD], resultNumbers[i][WOOD_SPENT_INDEX], BASE_TEN_NUMER);
    }
    game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
    if(context->gameResult == GAME_RESULT_VICTORY) {
        game_snd_play_music(GAME_MUSIC_VICTORY);
    } else if(context->gameResult == GAME_RESULT_DEFEAT) {
        game_snd_play_music(GAME_MUSIC_DEFEAT);
    }
	video_fade_in_init(DEFAULT_FADE_SPEED, context->mainPalette);
    goTitle = FALSE;
}

GameStateEnum handle_results_update(GameContext *context) {
    game_gui_handle(context, &guiScreen);
    if(goTitle) {
        video_fade_out_init(DEFAULT_FADE_SPEED);
        return GAME_STATE_TITLE;
    }
    return GAME_STATE_RESULTS;
}

void handle_results_render(GameContext *context, RenderQueue *renderQueue) {
    game_gui_render_queue_submit(context, renderQueue, &guiScreen);
    render_queue_submit_mouse(context, renderQueue);
}

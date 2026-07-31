#include <stdio.h>
#include "game/state/results.h"
#include "game/gui/gui.h"
#include "game/video/game_video.h"
#include "game/video/render.h"
#include "game/mouse/game_mouse.h"
#include "game/sound/game_sound.h"
#include "game/map/map_code.h"

#define RESULTS_BACKGROUND_PATH "assets/gfx/ui/back/results.pcx"
#define CONTROLLER_STATS 8
#define CONTROLLER_STAT_MAX_LENGTH 16
#define UNITS_TRAINED_INDEX 0
#define ENEMIES_KILLED_INDEX 1
#define BUILDINGS_CONSTRUCTED_INDEX 2
#define BUILDINGS_DESTROYED_INDEX 3
#define GOLD_GATHERED_INDEX 4
#define WOOD_GATHERED_INDEX 5
#define GOLD_SPENT_INDEX 6
#define WOOD_SPENT_INDEX 7
#define RESULT_X_MID 55
#define RESULT_X_INC (GAME_INTERNAL_WIDTH - 2 * RESULT_X_MID)
#define RESULT_Y_MIN 50
#define RESULT_Y_INC 15
#define RESULT_Z 10

static BITMAP *resultsBackground;

typedef struct {
    int color;
    char text[CONTROLLER_STAT_MAX_LENGTH];
    int length;
} ResultNumber;

static ResultNumber resultNumbers[UNIT_CONTROLLERS_COUNT][CONTROLLER_STATS];
static uint8_t goTitle;
static char resultTitle[64];

static void return_to_title(GameContext *context) {
    goTitle = TRUE;
}

static char* get_result_text(const GameContext *context) {
    return resultTitle;
}

#define MENU_ELEMENTS 9
#define MENU_BUTTON_WIDTH 120
#define MENU_BUTTON_HEIGHT 18
#define MENU_BUTTON_X 100
#define MENU_BUTTON_Y 175
#define MENU_STAT_Y 25
#define MENU_STAT_Y_INC 8

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
        .hotkey = KEY_T,
        .typed = {
            .button = {
                .size = { .width = MENU_BUTTON_WIDTH, .height = MENU_BUTTON_HEIGHT },
                .action = return_to_title
            }
        }
    },
	{
		.x = 0, .y = 10, .z = 5,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .customText = { .text = get_result_text, .maxX = 320 } }
	},
	{
		.x = RESULT_X_MID / 2, .y = MENU_STAT_Y, .z = 5,
		.type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_RESULT_YOU,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = RESULT_X_MID + RESULT_X_MID / 2 } }
	},
	{
		.x = RESULT_X_MID / 2, .y = MENU_STAT_Y + MENU_STAT_Y_INC, .z = 5,
		.type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_RESULT_YOU_UNDER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = RESULT_X_MID + RESULT_X_MID / 2 } }
	},
	{
		.x = RESULT_X_MID / 2 + RESULT_X_INC, .y = MENU_STAT_Y, .z = 5,
		.type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_RESULT_COMPUTER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = RESULT_X_MID + RESULT_X_MID / 2 + RESULT_X_INC } }
	},
    {
        .x = RESULT_X_MID / 2 + RESULT_X_INC, .y = MENU_STAT_Y + MENU_STAT_Y_INC, .z = 5,
        .type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_RESULT_COMPUTER_UNDER,
        .textColor = PAL_COLOR_WHITE,
        .shadowTextColor = PAL_COLOR_BLACK,
        .textBackground = TRANSPARENT_INDEX,
        .typed = { .text = { .maxX = RESULT_X_MID + RESULT_X_MID / 2 + RESULT_X_INC } }
    },
	{
		.x = RESULT_X_MID, .y = MENU_STAT_Y, .z = 5,
		.type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_RESULT_STATS,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = RESULT_X_MID + RESULT_X_INC } }
	},
	{
		.x = RESULT_X_MID, .y = MENU_STAT_Y + MENU_STAT_Y_INC, .z = 5,
		.type = GUI_ELEMENT_TEXT,
        .textId = GAME_TEXT_ID_RESULT_STATS_UNDER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = { .text = { .maxX = RESULT_X_MID + RESULT_X_INC } }
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
        int otherController = (i + 1) % UNIT_CONTROLLERS_COUNT;
        if(context->stats[i].unitsTrained < context->stats[otherController].unitsTrained) {
            resultNumbers[i][UNITS_TRAINED_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].unitsTrained > context->stats[otherController].unitsTrained) {
            resultNumbers[i][UNITS_TRAINED_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][UNITS_TRAINED_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].unitsTrained, resultNumbers[i][UNITS_TRAINED_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][UNITS_TRAINED_INDEX].length = text_length(context->gameFont, resultNumbers[i][UNITS_TRAINED_INDEX].text);
        
        if(context->stats[i].enemiesKilled < context->stats[otherController].enemiesKilled) {
            resultNumbers[i][ENEMIES_KILLED_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].enemiesKilled > context->stats[otherController].enemiesKilled) {
            resultNumbers[i][ENEMIES_KILLED_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][ENEMIES_KILLED_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].enemiesKilled, resultNumbers[i][ENEMIES_KILLED_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][ENEMIES_KILLED_INDEX].length = text_length(context->gameFont, resultNumbers[i][ENEMIES_KILLED_INDEX].text);
        
        if(context->stats[i].buildingsConstructed < context->stats[otherController].buildingsConstructed) {
            resultNumbers[i][BUILDINGS_CONSTRUCTED_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].buildingsConstructed > context->stats[otherController].buildingsConstructed) {
            resultNumbers[i][BUILDINGS_CONSTRUCTED_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][BUILDINGS_CONSTRUCTED_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].buildingsConstructed, resultNumbers[i][BUILDINGS_CONSTRUCTED_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][BUILDINGS_CONSTRUCTED_INDEX].length = text_length(context->gameFont, resultNumbers[i][BUILDINGS_CONSTRUCTED_INDEX].text);
        
        if(context->stats[i].buildingsDestroyed < context->stats[otherController].buildingsDestroyed) {
            resultNumbers[i][BUILDINGS_DESTROYED_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].buildingsDestroyed > context->stats[otherController].buildingsDestroyed) {
            resultNumbers[i][BUILDINGS_DESTROYED_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][BUILDINGS_DESTROYED_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].buildingsDestroyed, resultNumbers[i][BUILDINGS_DESTROYED_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][BUILDINGS_DESTROYED_INDEX].length = text_length(context->gameFont, resultNumbers[i][BUILDINGS_DESTROYED_INDEX].text);
        
        if(context->stats[i].resourcesGathered[RESOURCE_TYPE_GOLD] < context->stats[otherController].resourcesGathered[RESOURCE_TYPE_GOLD]) {
            resultNumbers[i][GOLD_GATHERED_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].resourcesGathered[RESOURCE_TYPE_GOLD] > context->stats[otherController].resourcesGathered[RESOURCE_TYPE_GOLD]) {
            resultNumbers[i][GOLD_GATHERED_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][GOLD_GATHERED_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].resourcesGathered[RESOURCE_TYPE_GOLD], resultNumbers[i][GOLD_GATHERED_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][GOLD_GATHERED_INDEX].length = text_length(context->gameFont, resultNumbers[i][GOLD_GATHERED_INDEX].text);
        
        if(context->stats[i].resourcesGathered[RESOURCE_TYPE_WOOD] < context->stats[otherController].resourcesGathered[RESOURCE_TYPE_WOOD]) {
            resultNumbers[i][WOOD_GATHERED_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].resourcesGathered[RESOURCE_TYPE_WOOD] > context->stats[otherController].resourcesGathered[RESOURCE_TYPE_WOOD]) {
            resultNumbers[i][WOOD_GATHERED_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][WOOD_GATHERED_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].resourcesGathered[RESOURCE_TYPE_WOOD], resultNumbers[i][WOOD_GATHERED_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][WOOD_GATHERED_INDEX].length = text_length(context->gameFont, resultNumbers[i][WOOD_GATHERED_INDEX].text);
        
        if(context->stats[i].resourcesSpent[RESOURCE_TYPE_GOLD] < context->stats[otherController].resourcesSpent[RESOURCE_TYPE_GOLD]) {
            resultNumbers[i][GOLD_SPENT_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].resourcesSpent[RESOURCE_TYPE_GOLD] > context->stats[otherController].resourcesSpent[RESOURCE_TYPE_GOLD]) {
            resultNumbers[i][GOLD_SPENT_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][GOLD_SPENT_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].resourcesSpent[RESOURCE_TYPE_GOLD], resultNumbers[i][GOLD_SPENT_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][GOLD_SPENT_INDEX].length = text_length(context->gameFont, resultNumbers[i][GOLD_SPENT_INDEX].text);
        
        if(context->stats[i].resourcesSpent[RESOURCE_TYPE_WOOD] < context->stats[otherController].resourcesSpent[RESOURCE_TYPE_WOOD]) {
            resultNumbers[i][WOOD_SPENT_INDEX].color = PAL_COLOR_RED;
        } else if(context->stats[i].resourcesSpent[RESOURCE_TYPE_WOOD] > context->stats[otherController].resourcesSpent[RESOURCE_TYPE_WOOD]) {
            resultNumbers[i][WOOD_SPENT_INDEX].color = PAL_COLOR_GREEN;
        } else {
            resultNumbers[i][WOOD_SPENT_INDEX].color = PAL_COLOR_GRAY;
        }
		itoa(context->stats[i].resourcesSpent[RESOURCE_TYPE_WOOD], resultNumbers[i][WOOD_SPENT_INDEX].text, BASE_TEN_NUMBER);
		resultNumbers[i][WOOD_SPENT_INDEX].length = text_length(context->gameFont, resultNumbers[i][WOOD_SPENT_INDEX].text);
    }
    game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
    if(context->gameResult == GAME_RESULT_VICTORY) {
        game_snd_play_music(GAME_MUSIC_VICTORY);
    } else if(context->gameResult == GAME_RESULT_DEFEAT) {
        game_snd_play_music(GAME_MUSIC_DEFEAT);
    }

    char* resultText;
    if(context->gameResult == GAME_RESULT_VICTORY) {
        resultText = (char *) text_get_by_id(GAME_TEXT_ID_RESULT_VICTORY);
    } else if(context->gameResult == GAME_RESULT_DEFEAT) {
        resultText = (char *) text_get_by_id(GAME_TEXT_ID_RESULT_DEFEAT);
    } else {
        resultText = "";
    }
    sprintf(resultTitle, resultText, context->map.title);
    timer_set_speed(TIMER_SPEED_NORMAL);

    if(context->gameResult == GAME_RESULT_VICTORY) {
        char fileFolder[512];
        get_parent_directory(context->mapPath, fileFolder, strlen(context->mapPath));
        MapCodes mapCodes = (MapCodes) { .codes= & context->map.winCode, 1 };
        map_code_merge_all(fileFolder, &mapCodes);
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
    for(int i = 0; i < UNIT_CONTROLLERS_COUNT; i++) {
        for(int j = 0; j < CONTROLLER_STATS; j++) {
            int x = RESULT_X_MID - (resultNumbers[i][j].length / 2) + (i * RESULT_X_INC);
            int y = RESULT_Y_MIN + (j * RESULT_Y_INC);
            render_queue_submit_text(renderQueue, RESULT_Z , context->gameFont,
                resultNumbers[i][j].text, x, y, resultNumbers[i][j].color, TRANSPARENT_INDEX);
        }
    }
    for(int i = GAME_TEXT_ID_RESULT_UNITS_TRAINED; i <= GAME_TEXT_ID_RESULT_WOOD_SPENT; i++) {
        char* statText = (char*) text_get_by_id(i);
        int x = RESULT_X_MID + RESULT_X_INC / 2 - (text_length(context->gameFont, statText) / 2);
        int y = RESULT_Y_MIN + ((i - GAME_TEXT_ID_RESULT_UNITS_TRAINED) * RESULT_Y_INC);
        render_queue_submit_text(renderQueue, RESULT_Z , context->gameFont,
            statText, x, y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
    }
}

#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include "game/scenario/scenario_select_map.h"
#include <allegro.h>

#define SCENARIO_SELECT_BACKGROUND_PATH "assets/gfx/ui/back/select.pcx"
#define SCENARIO_SELECT_MAPS 10
#define GUI_BODY_Y 20
#define GUI_BAR_X 152
#define GUI_ROWS_X 5
#define GUI_ROWS_WIDTH 140
#define GUI_DESCRIPTION_X 168
#define GUI_DESCRIPTION_MAX_WIDTH 140
#define GUI_DESCRIPTION_MAX_HEIGHT 160
#define GUI_DESCRIPTION_MAX_HEIGHT_SMALL (GUI_DESCRIPTION_MAX_HEIGHT / 2)
#define BUTTON_RETURN_Y 175
#define BUTTON_PLAY_Y 145

#define LEFT_BACK_X (GUI_ROWS_X - 2)
#define LEFT_BACK_Y (GUI_BODY_Y - 2)
#define LEFT_BACK_WIDTH (GUI_DESCRIPTION_MAX_WIDTH + 8)
#define LEFT_BACK_HEIGHT 122
#define RIGHT_BACK_WIDTH (GUI_DESCRIPTION_MAX_WIDTH + 4)
#define RIGHT_BACK_HEIGHT (GUI_DESCRIPTION_MAX_HEIGHT + 4)
#define RIGHT_BACK_HEIGHT_SMALL (GUI_DESCRIPTION_MAX_HEIGHT_SMALL + 4)
#define RIGHT_BACK_X (GUI_DESCRIPTION_X - 2)
#define RIGHT_BACK_Y (GUI_BODY_Y - 2)

typedef enum {
	SCENARIO_SELECT_BROWSE_STATE,
	SCENARIO_SELECT_RELOAD_STATE,
	SCENARIO_SELECT_GO_STATE,
	SCENARIO_SELECT_TITLE_STATE,
} ScenarioSelectStateEnum;

static ScenarioSelectStateEnum state;
static BITMAP *background;
static MapList *mapList = NULL;
static char *currentFolder;

static uint8_t scenarioSelectOffset;
static uint8_t scenarioSelected;

static uint8_t get_max_scenario(const GameContext *context) {
	return mapList->count - 1;
}

static uint8_t get_scenario_select_offset(const GameContext *context) {
	return scenarioSelectOffset;
}

static void set_scenario_select_offset(GameContext *context, uint8_t value) {
	scenarioSelectOffset = value;
}

static uint8_t get_min_scenario_select_offset(const GameContext *context) {
	return 0;
}

static uint8_t get_max_scenario_select_offset(const GameContext *context) {
	return max_val(0, mapList->count - SCENARIO_SELECT_MAPS);
}

static uint8_t get_max_scenario_selected(const GameContext *context) {
	return scenarioSelected;
}

static void set_scenario_selected(GameContext *context, uint8_t value) {
	if(value < mapList->count) scenarioSelected = value;
}

static char * get_scenario_text(const GameContext *context, uint8_t index) {
	if(index >= mapList->count) return NULL;
	return mapList->entries[index].title;
}

static BITMAP * get_scenario_icon(const GameContext *context, uint8_t index) {
	if(index >= mapList->count) return NULL;
	switch (mapList->entries[index].type) {
		case MAP_ENTRY_FOLDER: return game_gfx_get_icon(GAME_ICON_FOLDER);
		case MAP_ENTRY_FILE: return game_gfx_get_icon(GAME_ICON_MAP);
		case MAP_ENTRY_FOLDER_UP: return game_gfx_get_icon(GAME_ICON_FOLDER_UP);
	}
	return NULL;
}

static char * get_scenario_selected_description(const GameContext *context) {
	if(scenarioSelected >= mapList->count) return NULL;
	return mapList->entries[scenarioSelected].description;
}

static void folders_action(GameContext *context) {
	switch (mapList->entries[scenarioSelected].type) {
		case MAP_ENTRY_FOLDER_UP:
		case MAP_ENTRY_FOLDER: {
			state = SCENARIO_SELECT_RELOAD_STATE;
			break;
		}
		default: {
			break;
		}
	}
}

static void select_scenario_action(GameContext *context) {
	if(mapList->entries[scenarioSelected].type == MAP_ENTRY_FILE) state = SCENARIO_SELECT_GO_STATE;
}

static void go_title_action(GameContext *context) {
	state = SCENARIO_SELECT_TITLE_STATE;
}

#define SCENARIO_SELECT_ELEMENTS 12
#define RIGHT_TEXT_BACK_TEXT_INDEX 5
#define RIGHT_TEXT_BACK_RECT_INDEX 10
#define RIGHT_TEXT_BACK_RECT_FILL_INDEX 11

static GuiElement scenarioSelect[SCENARIO_SELECT_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &background } }
	},
	{
		.x = LEFT_BACK_X, .y = GUI_BODY_Y - 15, .z = 1,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_SELECT_SCENARIO,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	{
		.x = RIGHT_BACK_X, .y = GUI_BODY_Y - 15, .z = 1,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_SCENARIO_DESCRIPTION,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	{
		.x = GUI_BAR_X, .y = GUI_BODY_Y, .z = 10,
		.type = GUI_ELEMENT_VERTICAL_BAR,
		.typed = {
			.bar = {
				.getMaxValue = get_max_scenario_select_offset,
				.getMinValue = get_min_scenario_select_offset,
				.getValue = get_scenario_select_offset,
				.setValue = set_scenario_select_offset,
				.valueInc = 1,
			}
		}
	},
	{
		.x = GUI_ROWS_X, .y = GUI_BODY_Y, .z = 10,
		.type = GUI_ELEMENT_CUSTOM_TEXT_ROWS,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.customTextRows = {
				.getOffsetValue = get_scenario_select_offset,
				.getSelectedValue = get_max_scenario_selected,
				.setSelectedValue = set_scenario_selected,
				.getText = get_scenario_text,
				.getMaxRow = get_max_scenario,
				.getIcon = get_scenario_icon,
				.rowAction = folders_action,
				.ySeparation = 12,
				.width = GUI_ROWS_WIDTH,
				.numRows = SCENARIO_SELECT_MAPS,
				.selectedTextColor = PAL_COLOR_YELLOW
			}
		}
	},
	[RIGHT_TEXT_BACK_TEXT_INDEX] = {
		.x = GUI_DESCRIPTION_X, .y = GUI_BODY_Y, .z = 10,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.customText = {
				.text = get_scenario_selected_description,
				.maxHeight = GUI_DESCRIPTION_MAX_HEIGHT,
				.maxWidth = GUI_DESCRIPTION_MAX_WIDTH
			}
		}
	},
	{
		.x = LEFT_BACK_X, .y = BUTTON_RETURN_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_R,
		.typed = {
			.button = {
				.size = { .width = LEFT_BACK_WIDTH, .height = BUTTON_HEIGHT },
				.action = go_title_action
			}
		}
	},
	{
		.x = LEFT_BACK_X, .y = BUTTON_PLAY_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_START_GAME,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_A,
		.typed = {
			.button = {
				.size = { .width = LEFT_BACK_WIDTH, .height = BUTTON_HEIGHT },
				.action = select_scenario_action
			}
		}
	},
	{
		.type = GUI_ELEMENT_RECTANGLE,
		.x = LEFT_BACK_X, .y = LEFT_BACK_Y, .z = 9,
		.typed = {
			.rectangle = {
				.size = { .width = LEFT_BACK_WIDTH, .height = LEFT_BACK_HEIGHT },
				.color = PAL_COLOR_BLACK
			}
		}
	},
	{
		.type = GUI_ELEMENT_FILL_RECTANGLE,
		.x = LEFT_BACK_X, .y = LEFT_BACK_Y, .z = 8,
		.typed = {
			.fillRectangle = {
				.size = { .width = LEFT_BACK_WIDTH, .height = LEFT_BACK_HEIGHT },
				.color = PAL_COLOR_DARK_TURQUOISE
			}
		}
	},
	[RIGHT_TEXT_BACK_RECT_INDEX] = {
		.type = GUI_ELEMENT_RECTANGLE,
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y, .z = 9,
		.typed = {
			.rectangle = {
				.size = { .width = RIGHT_BACK_WIDTH, .height = RIGHT_BACK_HEIGHT },
				.color = PAL_COLOR_BLACK
			}
		}
	},
	[RIGHT_TEXT_BACK_RECT_FILL_INDEX] = {
		.type = GUI_ELEMENT_FILL_RECTANGLE,
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y, .z = 8,
		.typed = {
			.fillRectangle = {
				.size = { .width = RIGHT_BACK_WIDTH, .height = RIGHT_BACK_HEIGHT },
				.color = PAL_COLOR_DARK_TURQUOISE
			}
		}
	}
};

GuiScreen scenarioSelectGuiScreen = { .elements = scenarioSelect, .elementsCount = SCENARIO_SELECT_ELEMENTS };

static void resize_scenario_text_area() {
	if(mapList->count > 0) {
		if(mapList->entries[scenarioSelected].type == MAP_ENTRY_FOLDER) {
			scenarioSelect[RIGHT_TEXT_BACK_RECT_INDEX].typed.rectangle.size.height = RIGHT_BACK_HEIGHT;
			scenarioSelect[RIGHT_TEXT_BACK_RECT_FILL_INDEX].typed.fillRectangle.size.height = RIGHT_BACK_HEIGHT;
			scenarioSelect[RIGHT_TEXT_BACK_TEXT_INDEX].typed.customText.maxHeight = GUI_DESCRIPTION_MAX_HEIGHT;
		} else {
			scenarioSelect[RIGHT_TEXT_BACK_RECT_INDEX].typed.rectangle.size.height = RIGHT_BACK_HEIGHT_SMALL;
			scenarioSelect[RIGHT_TEXT_BACK_RECT_FILL_INDEX].typed.fillRectangle.size.height = RIGHT_BACK_HEIGHT_SMALL;
			scenarioSelect[RIGHT_TEXT_BACK_TEXT_INDEX].typed.customText.maxHeight = GUI_DESCRIPTION_MAX_HEIGHT_SMALL;
		}
	}
}

static void free_all() {
	destroy_bitmap(background);
	background = NULL;
	scenario_select_free_maps(mapList);
	mapList = NULL;
	free(currentFolder);
	currentFolder = NULL;
}

static void init_scenarios_folder(char *folder) {
	if (currentFolder) free(currentFolder);
	currentFolder = strdup(folder);
	if (!currentFolder) {
		TRACE("Error allocating current folder.\n");
		exit(PROGRAM_ERROR);
	}
	if (scenario_select_load_maps(currentFolder, &mapList) != 0) {
		TRACE("Error loading maps from %s\n", currentFolder);
	} else if (mapList && mapList->count > 0) {
		TRACE("Loaded %d map files\n", mapList->count);
		for (int i = 0; i < mapList->count; i++) {
			const char *type = (mapList->entries[i].type == MAP_ENTRY_FOLDER) ? "DIR" : "FILE";
			const char *title = mapList->entries[i].title ? mapList->entries[i].title : "(no title)";
			const char *desc = mapList->entries[i].description ? mapList->entries[i].description : "(no description)";
			TRACE("  [%d] %s (%s) - Title: '%s' - Desc: '%s'\n", i, mapList->entries[i].path, type, title, desc);
		}
	} else {
		TRACE("No .fgm files found in %s\n", MAPS_FOLDER);
	}

	scenarioSelectOffset = 0;
	scenarioSelected = 0;
	state = SCENARIO_SELECT_BROWSE_STATE;
	resize_scenario_text_area();
}

void handle_scenario_select_render(GameContext *context, RenderQueue *renderQueue) {
	game_gui_render_queue_submit(context, renderQueue, &scenarioSelectGuiScreen);
	render_queue_submit_mouse(context, renderQueue);
}

void handle_scenario_select_init(GameContext *context) {
	background = load_bitmap(SCENARIO_SELECT_BACKGROUND_PATH, NULL);
	if (!background) {
		TRACE("Error loading scenario select background bitmap: %s\n", SCENARIO_SELECT_BACKGROUND_PATH);
		exit(PROGRAM_ERROR);
	}
	
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	init_scenarios_folder(MAPS_FOLDER);
	video_fade_in_init(DEFAULT_FADE_SPEED, context->mainPalette);
}

GameStateEnum handle_scenario_select_update(GameContext *context) {
	resize_scenario_text_area();
	if(mapList->count > 0) {
		if(mapList->entries[scenarioSelected].type == MAP_ENTRY_FOLDER) {
			scenarioSelect[RIGHT_TEXT_BACK_RECT_INDEX].typed.rectangle.size.height = RIGHT_BACK_HEIGHT;
			scenarioSelect[RIGHT_TEXT_BACK_RECT_FILL_INDEX].typed.fillRectangle.size.height = RIGHT_BACK_HEIGHT;
		} else {
			scenarioSelect[RIGHT_TEXT_BACK_RECT_INDEX].typed.rectangle.size.height = RIGHT_BACK_HEIGHT_SMALL;
			scenarioSelect[RIGHT_TEXT_BACK_RECT_FILL_INDEX].typed.fillRectangle.size.height = RIGHT_BACK_HEIGHT_SMALL;
		}
	}
	game_gui_handle(context, &scenarioSelectGuiScreen);
	
	// Load test map as default for now
	switch (state) {
		case SCENARIO_SELECT_GO_STATE: {
			free(context->mapPath);
			context->mapPath = strdup(mapList->entries[scenarioSelected].path);
			free_all();
			video_fade_out_init(DEFAULT_FADE_SPEED);
			return GAME_STATE_LOAD_MAP;
			break;
		}
		case SCENARIO_SELECT_RELOAD_STATE: {
			init_scenarios_folder(mapList->entries[scenarioSelected].path);
			state = SCENARIO_SELECT_BROWSE_STATE;
			break;
		}
		case SCENARIO_SELECT_TITLE_STATE: {
			free_all();
			video_fade_out_init(DEFAULT_FADE_SPEED);
			return GAME_STATE_TITLE;
		}
		default:
			break;
	}

	return GAME_STATE_SCENARIO_SELECT;
}

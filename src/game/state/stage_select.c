#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include "game/state/stage_select_map.h"
#include <allegro.h>

#define STAGE_SELECT_BACKGROUND_PATH "assets/ui/back_f.pcx"
#define STAGE_SELECT_MAPS 10
#define GUI_BODY_Y 20
#define GUI_BAR_X 150
#define GUI_ROWS_X 5
#define GUI_ROWS_WIDTH 140
#define GUI_DESCRIPTION_X 163
#define GUI_DESCRIPTION_MAX_WIDTH 157
#define GUI_DESCRIPTION_MAX_HEIGHT 180
#define BUTTON_RETURN_WIDTH 50
#define BUTTON_RETURN_X 110
#define BUTTON_RETURN_Y 170

#define BUTTON_PLAY_WIDTH 50
#define BUTTON_PLAY_X 50
#define BUTTON_PLAY_Y 170

typedef enum {
	STAGE_SELECT_BROWSE_STATE,
	STAGE_SELECT_RELOAD_STATE,
	STAGE_SELECT_GO_STATE,
	STAGE_SELECT_TITLE_STATE,
} StageSelectStateEnum;

static StageSelectStateEnum state;
static BITMAP *background;
static MapList *mapList = NULL;
static char *currentFolder;

static uint8_t stageSelectOffset;
static uint8_t stageSelected;
static char fpsText[16];

static uint8_t get_max_stage(const GameContext *context) {
	return mapList->count - 1;
}

static uint8_t get_stage_select_offset(const GameContext *context) {
	return stageSelectOffset;
}

static void set_stage_select_offset(GameContext *context, uint8_t value) {
	stageSelectOffset = value;
}

static uint8_t get_min_stage_select_offset(const GameContext *context) {
	return 0;
}

static uint8_t get_max_stage_select_offset(const GameContext *context) {
	return max_val(0, mapList->count - 1 - STAGE_SELECT_MAPS);
}

static uint8_t get_max_stage_selected(const GameContext *context) {
	return stageSelected;
}

static void set_stage_selected(GameContext *context, uint8_t value) {
	if(value < mapList->count) stageSelected = value;
}

static char * get_stage_text(const GameContext *context, uint8_t index) {
	if(index >= mapList->count) return NULL;
	return mapList->entries[index].title;
}

static BITMAP * get_stage_icon(const GameContext *context, uint8_t index) {
	if(index >= mapList->count) return NULL;
	switch (mapList->entries[index].type) {
		case MAP_ENTRY_FOLDER: return game_gfx_get_icon(GAME_ICON_FOLDER);
		case MAP_ENTRY_FILE: return game_gfx_get_icon(GAME_ICON_MAP);
		case MAP_ENTRY_FOLDER_UP: return game_gfx_get_icon(GAME_ICON_FOLDER_UP);
	}
	return NULL;
}

static char * get_stage_selected_description(const GameContext *context) {
	if(stageSelected >= mapList->count) return NULL;
	return mapList->entries[stageSelected].description;
}

static void folders_action(GameContext *context) {
	switch (mapList->entries[stageSelected].type) {
		case MAP_ENTRY_FOLDER_UP:
		case MAP_ENTRY_FOLDER: {
			state = STAGE_SELECT_RELOAD_STATE;
			break;
		}
		default: {
			break;
		}
	}
}

static void select_stage_action(GameContext *context) {
	if(mapList->entries[stageSelected].type == MAP_ENTRY_FILE) state = STAGE_SELECT_GO_STATE;
}

static void go_title_action(GameContext *context) {
	state = STAGE_SELECT_TITLE_STATE;
}

#define STAGE_SELECT_ELEMENTS 6

static GuiElement stageSelect[STAGE_SELECT_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &background } }
	},
	{
		.x = GUI_BAR_X, .y = GUI_BODY_Y, .z = 10,
		.type = GUI_ELEMENT_VERTICAL_BAR,
		.typed = {
			.bar = {
				.getMaxValue = get_max_stage_select_offset,
				.getMinValue = get_min_stage_select_offset,
				.getValue = get_stage_select_offset,
				.setValue = set_stage_select_offset,
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
				.getOffsetValue = get_stage_select_offset,
				.getSelectedValue = get_max_stage_selected,
				.setSelectedValue = set_stage_selected,
				.getText = get_stage_text,
				.getMaxRow = get_max_stage,
				.getIcon = get_stage_icon,
				.rowAction = folders_action,
				.ySeparation = 12,
				.width = GUI_ROWS_WIDTH,
				.numRows = STAGE_SELECT_MAPS,
				.selectedTextColor = PAL_COLOR_YELLOW
			}
		}
	},
	{
		.x = GUI_DESCRIPTION_X, .y = GUI_BODY_Y, .z = 10,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.customText = {
				.text = get_stage_selected_description,
				.maxHeight = GUI_DESCRIPTION_MAX_HEIGHT,
				.maxWidth = GUI_DESCRIPTION_MAX_WIDTH
			}
		}
	},
	{
		.x = BUTTON_RETURN_X, .y = BUTTON_RETURN_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_RETURN,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_T,
		.typed = {
			.button = {
				.size = { .width = BUTTON_RETURN_WIDTH, .height = BUTTON_HEIGHT },
				.action = go_title_action
			}
		}
	},
	{
		.x = BUTTON_PLAY_X, .y = BUTTON_PLAY_Y, .z = 10,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_START_GAME,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_A,
		.typed = {
			.button = {
				.size = { .width = BUTTON_PLAY_WIDTH, .height = BUTTON_HEIGHT },
				.action = select_stage_action
			}
		}
	},
};

GuiScreen stageSelectGuiScreen = { .elements = stageSelect, .elementsCount = STAGE_SELECT_ELEMENTS };

static void free_all() {
	destroy_bitmap(background);
	background = NULL;
	stage_select_free_maps(mapList);
	mapList = NULL;
	free(currentFolder);
	currentFolder = NULL;
}

static void init_stages_folder(char *folder) {
	if (currentFolder) free(currentFolder);
	currentFolder = strdup(folder);
	if (!currentFolder) {
		TRACE("Error allocating current folder.\n");
		exit(PROGRAM_ERROR);
	}
	if (stage_select_load_maps(currentFolder, &mapList) != 0) {
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

	stageSelectOffset = 0;
	stageSelected = 0;
	state = STAGE_SELECT_BROWSE_STATE;
}

GameStateEnum handle_init_stage_select(GameContext *context, RenderQueue *renderQueue) {
	background = load_bitmap(STAGE_SELECT_BACKGROUND_PATH, NULL);
	if (!background) {
		TRACE("Error loading stage select background bitmap: %s\n", STAGE_SELECT_BACKGROUND_PATH);
		exit(PROGRAM_ERROR);
	}
	
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	init_stages_folder(MAPS_FOLDER);
	
	return GAME_STATE_STAGE_SELECT;
}

GameStateEnum handle_stage_select(GameContext *context, RenderQueue *renderQueue) {
	game_gui_handle(context, &stageSelectGuiScreen);
	
	// Load test map as default for now
	switch (state) {
		case STAGE_SELECT_GO_STATE: {
			free(context->mapPath);
			context->mapPath = strdup(mapList->entries[stageSelected].path);
			free_all();
			return GAME_STATE_LOAD_MAP;
			break;
		}
		case STAGE_SELECT_RELOAD_STATE: {
			init_stages_folder(mapList->entries[stageSelected].path);
			state = STAGE_SELECT_BROWSE_STATE;
			break;
		}
		case STAGE_SELECT_TITLE_STATE: {
			free_all();
			return GAME_STATE_INIT_TITLE;
		}
		default:
			break;
	}

	game_gui_render_queue_submit(context, renderQueue, &stageSelectGuiScreen);
	render_queue_submit_mouse(context, renderQueue);
	snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
	render_queue_submit_text(renderQueue, UI_Z_ORDER + 510, context->gameFont, fpsText, 260, 180, PAL_COLOR_WHITE, -1);

	return GAME_STATE_STAGE_SELECT;
}

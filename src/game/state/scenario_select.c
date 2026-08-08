#include <stdio.h>
#include <stdlib.h>
#include "game/state/scenario_select.h"
#include "game/scenario/scenario_select_map.h"
#include "game/gui/gui.h"
#include "game/video/game_video.h"
#include "game/video/gfx.h"
#include "game/video/render.h"
#include "game/mouse/game_mouse.h"
#include "game/sound/game_sound.h"

#define SCENARIO_SELECT_BACKGROUND_PATH "assets/gfx/ui/back/select.pcx"
#define SCENARIO_SELECT_MAPS 10
#define GUI_BODY_Y 20
#define GUI_BAR_X 152
#define GUI_ROWS_X 5
#define GUI_ROWS_WIDTH 140
#define GUI_DESCRIPTION_X 168
#define GUI_DESCRIPTION_MAX_WIDTH 140
#define GUI_DESCRIPTION_MAX_HEIGHT 145
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
	if(value < mapList->count && mapList->entries[value].unlocked) scenarioSelected = value;
}

static char * get_scenario_text(const GameContext *context, uint8_t index) {
	if(index >= mapList->count) return NULL;
	if(mapList->entries[index].unlocked) {
		return mapList->entries[index].title;
	}
	else {
		return (char *) text_get_by_id(GAME_TEXT_ID_UNKNOWN);
	}
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
	if(mapList->entries[scenarioSelected].unlocked) {
		return mapList->entries[scenarioSelected].description;
	} else {
		return (char *) text_get_by_id(GAME_TEXT_ID_UNKNOWN);
	}
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
	if(mapList->entries[scenarioSelected].type == MAP_ENTRY_FILE) {
		state = SCENARIO_SELECT_GO_STATE;
	}
	else {
		game_snd_play_sound(GAME_SOUND_NOT_VALID);
	}
}

static void go_title_action(GameContext *context) {
	state = SCENARIO_SELECT_TITLE_STATE;
}

#define SCENARIO_SELECT_ELEMENTS 9

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
};

GuiScreen scenarioSelectGuiScreen = { .elements = scenarioSelect, .elementsCount = SCENARIO_SELECT_ELEMENTS };

#define SCENARIO_SELECT_FOLDER_ELEMENTS 3
static GuiElement scenarioSelectFolder[SCENARIO_SELECT_FOLDER_ELEMENTS] = {
	{
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
		.type = GUI_ELEMENT_RECTANGLE,
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y, .z = 9,
		.typed = {
			.rectangle = {
				.size = { .width = RIGHT_BACK_WIDTH, .height = RIGHT_BACK_HEIGHT },
				.color = PAL_COLOR_BLACK
			}
		}
	},
	{
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

GuiScreen scenarioSelectFolderGuiScreen = { .elements = scenarioSelectFolder, .elementsCount = SCENARIO_SELECT_FOLDER_ELEMENTS };

#define SCENARIO_SELECT_MAP_ELEMENTS 21
#define MAP_OPTIONS_SEPARATOR 100
#define MAP_OPTIONS_SMALL_SEPARATOR 50
#define MAP_OPTIONS_Y_SEPARATION 2
#define UPGRADABLE_SOLDIER_INDEX 6
#define UPGRADABLE_ARCHER_INDEX 7
#define UPGRADABLE_KNIGHT_INDEX 8
#define UPGRADABLE_MAGE_INDEX 9
#define UPGRADADED_SOLDIER_INDEX 10
#define UPGRADADED_ARCHER_INDEX 11
#define UPGRADADED_KNIGHT_INDEX 12
#define UPGRADADED_MAGE_INDEX 13
#define CONSTRUCTABLE_CITY_HALL_INDEX 14
#define CONSTRUCTABLE_FARM_INDEX 15
#define CONSTRUCTABLE_BARRACKS_INDEX 16
#define CONSTRUCTABLE_BLACKSMITH_INDEX 17
#define CONSTRUCTABLE_STABLES_INDEX 18
#define CONSTRUCTABLE_TOWER_INDEX 19
#define CONSTRUCTABLE_TURRET_INDEX 20

static GuiElement scenarioSelectMap[SCENARIO_SELECT_MAP_ELEMENTS] = {
	{
		.x = GUI_DESCRIPTION_X, .y = GUI_BODY_Y, .z = 10,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.customText = {
				.text = get_scenario_selected_description,
				.maxHeight = GUI_DESCRIPTION_MAX_HEIGHT_SMALL,
				.maxWidth = GUI_DESCRIPTION_MAX_WIDTH
			}
		}
	},
	{
		.type = GUI_ELEMENT_RECTANGLE,
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y, .z = 9,
		.typed = {
			.rectangle = {
				.size = { .width = RIGHT_BACK_WIDTH, .height = RIGHT_BACK_HEIGHT_SMALL },
				.color = PAL_COLOR_BLACK
			}
		}
	},
	{
		.type = GUI_ELEMENT_FILL_RECTANGLE,
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y, .z = 8,
		.typed = {
			.fillRectangle = {
				.size = { .width = RIGHT_BACK_WIDTH, .height = RIGHT_BACK_HEIGHT_SMALL },
				.color = PAL_COLOR_DARK_TURQUOISE
			}
		}
	},
	{
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 30, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_UPGRADABLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
	},
	{
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 60, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_UPGRADED,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
	},
	{
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_BUILDING_CONSTRUCTIBLE,
		.textColor = PAL_COLOR_YELLOW,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
	},
	[UPGRADABLE_SOLDIER_INDEX] = {
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 40, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_SOLDIER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[UPGRADABLE_ARCHER_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SEPARATOR, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 40, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_ARCHER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[UPGRADABLE_KNIGHT_INDEX] = {
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 50, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_KNIGHT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[UPGRADABLE_MAGE_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SEPARATOR, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 50, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_MAGE,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[UPGRADADED_SOLDIER_INDEX] = {
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 70, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_SOLDIER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[UPGRADADED_ARCHER_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SEPARATOR, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 70, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_ARCHER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[UPGRADADED_KNIGHT_INDEX] = {
		.x = RIGHT_BACK_X, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 80, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_KNIGHT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
        },
	[UPGRADADED_MAGE_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SEPARATOR, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 80, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_MAGE,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[CONSTRUCTABLE_CITY_HALL_INDEX] = {
		.x = RIGHT_BACK_X - 3, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 10, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_CITY_HALL,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[CONSTRUCTABLE_FARM_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SMALL_SEPARATOR + 5, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 10, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_FARM,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[CONSTRUCTABLE_BARRACKS_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SMALL_SEPARATOR * 2 + 2, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 10, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_BARRACKS,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[CONSTRUCTABLE_BLACKSMITH_INDEX] = {
		.x = RIGHT_BACK_X - 3, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 20, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_BLACKSMITH,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[CONSTRUCTABLE_STABLES_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SMALL_SEPARATOR + 5, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 20, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_STABLES,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[CONSTRUCTABLE_TOWER_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SMALL_SEPARATOR * 2 + 2, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION + 20, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_TOWER,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},
	[CONSTRUCTABLE_TURRET_INDEX] = {
		.x = RIGHT_BACK_X + MAP_OPTIONS_SMALL_SEPARATOR * 2 + 2, .y = RIGHT_BACK_Y + RIGHT_BACK_HEIGHT_SMALL + MAP_OPTIONS_Y_SEPARATION, .z = 10,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_UNIT_TYPE_TURRET,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX
	},	
};

GuiScreen scenarioSelectMapGuiScreen = { .elements = scenarioSelectMap, .elementsCount = SCENARIO_SELECT_MAP_ELEMENTS };

static void update_map_options() {
	MapEntry *mapEntry = &mapList->entries[scenarioSelected];
	if(mapList->count > 0 && mapEntry->type == MAP_ENTRY_FILE) {
		for(int i = 0; i < MAP_UPGRADEABLE_UNIT_TYPES; i++) {
			uint8_t upgradeable = mapEntry->upgradeableUnits[i];
			uint8_t upgraded = mapEntry->upgradedUnits[i];
			scenarioSelectMap[UPGRADABLE_SOLDIER_INDEX + i].textColor = upgradeable ? PAL_COLOR_GREEN : PAL_COLOR_RED;
			scenarioSelectMap[UPGRADADED_SOLDIER_INDEX + i].textColor = upgraded ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		}
		scenarioSelectMap[CONSTRUCTABLE_CITY_HALL_INDEX].textColor = mapEntry->enableCityHall ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		scenarioSelectMap[CONSTRUCTABLE_FARM_INDEX].textColor = mapEntry->enableFarm ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		scenarioSelectMap[CONSTRUCTABLE_BARRACKS_INDEX].textColor = mapEntry->enableBarracks ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		scenarioSelectMap[CONSTRUCTABLE_BLACKSMITH_INDEX].textColor = mapEntry->enableBlacksmith ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		scenarioSelectMap[CONSTRUCTABLE_STABLES_INDEX].textColor = mapEntry->enableStables ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		scenarioSelectMap[CONSTRUCTABLE_TOWER_INDEX].textColor = mapEntry->enableTower ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		scenarioSelectMap[CONSTRUCTABLE_TURRET_INDEX].textColor = mapEntry->enableTurret ? PAL_COLOR_GREEN : PAL_COLOR_RED;
	}
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
	update_map_options();
}

void handle_scenario_select_render(GameContext *context, RenderQueue *renderQueue) {
	game_gui_render_queue_submit(context, renderQueue, &scenarioSelectGuiScreen);
	if(mapList->count > 0) {
		switch(mapList->entries[scenarioSelected].type) {
			case MAP_ENTRY_FOLDER:
			case MAP_ENTRY_FOLDER_UP:
				game_gui_render_queue_submit(context, renderQueue, &scenarioSelectFolderGuiScreen);
				break;
			case MAP_ENTRY_FILE:
				game_gui_render_queue_submit(context, renderQueue, &scenarioSelectMapGuiScreen);
				break;
		}
	}
	render_queue_submit_mouse(context, renderQueue);
}

void handle_scenario_select_init(GameContext *context) {
	background = load_bitmap(SCENARIO_SELECT_BACKGROUND_PATH, NULL);
	if (!background) {
		TRACE("Error loading scenario select background bitmap: %s\n", SCENARIO_SELECT_BACKGROUND_PATH);
		exit(PROGRAM_ERROR);
	}
	
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	if(context->mapFolderPath) {
		init_scenarios_folder(context->mapFolderPath);
		free(context->mapFolderPath);
		context->mapFolderPath = NULL;
	} else {
		init_scenarios_folder(MAPS_FOLDER);
	}
	video_fade_in_init(DEFAULT_FADE_SPEED, context->mainPalette);
}

GameStateEnum handle_scenario_select_update(GameContext *context) {
	update_map_options();
	game_gui_handle(context, &scenarioSelectGuiScreen);
	
	switch (state) {
		case SCENARIO_SELECT_GO_STATE: {
			free(context->mapPath);
			context->mapPath = strdup(mapList->entries[scenarioSelected].path);
			return GAME_STATE_LOAD_MAP;
			break;
		}
		case SCENARIO_SELECT_RELOAD_STATE: {
			init_scenarios_folder(mapList->entries[scenarioSelected].path);
			state = SCENARIO_SELECT_BROWSE_STATE;
			break;
		}
		case SCENARIO_SELECT_TITLE_STATE: {
			return GAME_STATE_TITLE;
		}
		default:
			break;
	}

	return GAME_STATE_SCENARIO_SELECT;
}

void handle_scenario_select_exit(GameContext *context) {
	if(background) {
		destroy_bitmap(background);
		background = NULL;
	}
	scenario_select_free_maps(mapList);
	mapList = NULL;
	if(currentFolder) {
		free(currentFolder);
		currentFolder = NULL;
	}
	video_fade_out_init(DEFAULT_FADE_SPEED);
}

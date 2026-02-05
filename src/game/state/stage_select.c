#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include "game/state/stage_select_map.h"
#include <allegro.h>

#define STAGE_SELECT_BACKGROUND_PATH "assets/ui/title.pcx"

static BITMAP *background;
static MapList *mapList = NULL;

#define STAGE_SELECT_ELEMENTS 3
#define STAGE_SELECT_MAPS 10
uint8_t stageSelectOffset;
uint8_t stageSelected;

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

static char * get_stage_text(const GameContext *context, uint8_t index) {
	if(index >= mapList->count) return NULL;
	return mapList->entries[index].title;
}

static GuiElement stageSelect[STAGE_SELECT_ELEMENTS] = {
	{
		.x = 0, .y = 0, .z = 0,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &background } }
	},
	{
		.x = 10, .y = 10, .z = 10,
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
		.x = 50 + 5, .y = 10, .z = 5,
		.type = GUI_ELEMENT_CUSTOM_TEXT_ROWS,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.customTextRows = {
				.getOffsetValue = get_stage_select_offset,
				.getSelectedValue = get_max_stage_selected,
				.getText = get_stage_text,
				.getMaxRow = get_max_stage,
				.maxX = 200,
				.ySeparation = 15,
				.numRows = STAGE_SELECT_MAPS,
				.selectedTextColor = PAL_COLOR_YELLOW
			}
		}
	},
};

GuiScreen stageSelectGuiScreen = { .elements = stageSelect, .elementsCount = STAGE_SELECT_ELEMENTS };

GameStateEnum handle_init_stage_select(GameContext *context, RenderQueue *renderQueue) {
	background = load_bitmap(STAGE_SELECT_BACKGROUND_PATH, NULL);
	if (!background) {
		fprintf(stderr, "Error loading stage select background bitmap: %s\n", STAGE_SELECT_BACKGROUND_PATH);
		exit(PROGRAM_ERROR);
	}
	
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);

	if (stage_select_load_maps(MAPS_FOLDER, &mapList) != 0) {
		fprintf(stderr, "Error loading maps from %s\n", MAPS_FOLDER);
	} else if (mapList && mapList->count > 0) {
		printf("Loaded %d map files\n", mapList->count);
		for (int i = 0; i < mapList->count; i++) {
			const char *type = (mapList->entries[i].type == MAP_ENTRY_DIRECTORY) ? "DIR" : "FILE";
			const char *title = mapList->entries[i].title ? mapList->entries[i].title : "(no title)";
			const char *desc = mapList->entries[i].description ? mapList->entries[i].description : "(no description)";
			printf("  [%d] %s (%s) - Title: '%s' - Desc: '%s'\n", i, mapList->entries[i].path, type, title, desc);
		}
	} else {
		printf("No .fgm files found in %s\n", MAPS_FOLDER);
	}

	stageSelectOffset = 0;
	stageSelected = 0;
	
	return GAME_STATE_STAGE_SELECT;
}

GameStateEnum handle_stage_select(GameContext *context, RenderQueue *renderQueue) {
	game_gui_handle(context, &stageSelectGuiScreen);
	
	// Load test map as default for now
	if (keyboard_is_key_pressed(KEY_SPACE)) {
        // TODO: use selected entry if it is not a dir
        free(context->mapPath);
		context->mapPath = strdup(mapList->entries[1].path);
		if (background) {
			destroy_bitmap(background);
			background = NULL;
		}
		if (mapList) {
			stage_select_free_maps(mapList);
			mapList = NULL;
		}
		return GAME_STATE_LOAD_MAP;
	}
	
	// Return to title
	if (keyboard_is_key_pressed(KEY_ESC)) {
		if (background) {
			destroy_bitmap(background);
			background = NULL;
		}
		if (mapList) {
			stage_select_free_maps(mapList);
			mapList = NULL;
		}
		return GAME_STATE_TITLE;
	}
	
	game_gui_render_queue_submit(context, renderQueue, &stageSelectGuiScreen);
	render_queue_submit_mouse(context, renderQueue);
	
	return GAME_STATE_STAGE_SELECT;
}

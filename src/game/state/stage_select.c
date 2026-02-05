#include <stdio.h>
#include <stdlib.h>
#include "game/game_lib.h"
#include "game/state/stage_select_map.h"
#include <allegro.h>

#define STAGE_SELECT_BACKGROUND_PATH "assets/ui/title.pcx"

static BITMAP *stageSelectBackground;
static MapList *mapList = NULL;

GameStateEnum handle_init_stage_select(GameContext *context, RenderQueue *renderQueue) {
	stageSelectBackground = load_bitmap(STAGE_SELECT_BACKGROUND_PATH, NULL);
	if (!stageSelectBackground) {
		fprintf(stderr, "Error loading stage select background bitmap: %s\n", STAGE_SELECT_BACKGROUND_PATH);
		return GAME_STATE_INIT_TITLE;
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

	// TODO Initialize GUI elements for stage selection
	
	return GAME_STATE_STAGE_SELECT;
}

GameStateEnum handle_stage_select(GameContext *context, RenderQueue *renderQueue) {
	// TODO Implement dynamic map listing and selection
	// For now, return to title on ESC or load test map
	
	// Load test map as default for now
	if (keyboard_is_key_pressed(KEY_SPACE)) {
        // TODO: use selected entry if it is not a dir
        free(context->mapPath);
		context->mapPath = strdup(mapList->entries[1].path);
		if (stageSelectBackground) {
			destroy_bitmap(stageSelectBackground);
			stageSelectBackground = NULL;
		}
		if (mapList) {
			stage_select_free_maps(mapList);
			mapList = NULL;
		}
		return GAME_STATE_LOAD_MAP;
	}
	
	// Return to title
	if (keyboard_is_key_pressed(KEY_ESC)) {
		if (stageSelectBackground) {
			destroy_bitmap(stageSelectBackground);
			stageSelectBackground = NULL;
		}
		if (mapList) {
			stage_select_free_maps(mapList);
			mapList = NULL;
		}
		return GAME_STATE_TITLE;
	}
	
	// Render stage select screen
	render_queue_submit_clear(renderQueue, 0, 0);
	
	// TODO Add stage select GUI elements
	
	return GAME_STATE_STAGE_SELECT;
}

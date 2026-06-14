#include <stdio.h>
#include <stdlib.h>
#include "game/state/play_map.h"
#include "game/ai/strategy_ai.h"
#include "game/gui/gui.h"
#include "game/gui/cmd_bar.h"
#include "game/map/spatial.h"
#include "game/map/selection.h"
#include "game/map/resource.h"
#include "game/animation/animation.h"
#include "game/building/building.h"
#include "game/object/object.h"
#include "game/unit/unit.h"
#include "game/unit/unit_command.h"
#include "game/unit/player_attacked.h"
#include "game/video/game_video.h"
#include "game/video/render.h"
#include "game/video/gfx.h"
#include "game/mouse/game_mouse.h"

#define RESULT_MESSAGE_X_OFFSET 5
#define TOOLTIP_TEXT_X_OFFSET 5
#define TOOLTIP_TEXT_Y_OFFSET 5
#define TOOLTIP_Z (UI_Z_ORDER + 1000)

static int moveViewportCounter = 0;
static GameStateEnum nextState;
static uint8_t renderBackground;
static char tooltipQtyBuffer[16];
static const char *tooltipText;
static uint8_t showResourceTooltip;
static int tooltipX, tooltipY;

static void go_menu(GameContext* context) {
	nextState = GAME_STATE_MENU_MAP;
	renderBackground = TRUE;
}

static void go_results(GameContext* context) {
	video_fade_out_init(DEFAULT_FADE_SPEED);
	nextState = GAME_STATE_RESULTS;
}

static char* get_win_text(const GameContext* context) {
	return context->map.win;
}

static char* get_lose_text(const GameContext* context) {
	return context->map.lose;
}

#define PLAY_MENU_ELEMENTS 1

static GuiElement playMenu[PLAY_MENU_ELEMENTS] = {
	{
		.x = 3, .y = 1, .z = UI_Z_ORDER + 900,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_MENU_PLAY,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.hotkey = KEY_F10,
		.typed = {
			.button = {
				.size = { .width = 66, .height = 9 },
				.action = go_menu,
				.fit = TRUE
			}
		}
	},
};

static BITMAP* menuBack;

#define BUTTON_CONFIRM_RESULT_WIDTH 80
#define BUTTON_CONFIRM_RESULT_X (MENU_BACK_X + (MENU_BACK_WIDTH - BUTTON_CONFIRM_RESULT_WIDTH) / 2)
#define BUTTON_CONFIRM_RESULT_Y (MENU_BACK_Y + MENU_BACK_HEIGHT - BUTTON_HEIGHT - 20)

#define WIN_MENU_ELEMENTS 4
#define MINIMAP_CENTER_OFFSET_X 8
#define MINIMAP_CENTER_OFFSET_Y 6

static GuiElement winMenu[WIN_MENU_ELEMENTS] = {
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y, .z = UI_Z_ORDER + 900,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &menuBack } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = UI_Z_ORDER + 901,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_RESULT_WIN_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = MENU_BACK_X + RESULT_MESSAGE_X_OFFSET, .y = MENU_BACK_Y + 3 * MENU_TITLE_Y_OFFSET, .z = UI_Z_ORDER + 902,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.customText = {
				.text = get_win_text,
				.maxHeight = MENU_BACK_HEIGHT  - 3 * MENU_TITLE_Y_OFFSET,
				.maxWidth = MENU_BACK_WIDTH - 2 * RESULT_MESSAGE_X_OFFSET
			}
		}
	},
	{
		.x = BUTTON_CONFIRM_RESULT_X, .y = BUTTON_CONFIRM_RESULT_Y, .z = UI_Z_ORDER + 903,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_RESULT_WIN_ACCEPT,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_V,
		.typed = {
			.button = {
				.size = { .width = BUTTON_CONFIRM_RESULT_WIDTH , .height = BUTTON_HEIGHT },
				.action = go_results
			}
		}
	},
};

#define LOSE_MENU_ELEMENTS 4

static GuiElement loseMenu[LOSE_MENU_ELEMENTS] = {
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y, .z = UI_Z_ORDER + 900,
		.type = GUI_ELEMENT_IMAGE,
		.typed = { .image = { .bitmap = &menuBack } }
	},
	{
		.x = MENU_BACK_X, .y = MENU_BACK_Y + MENU_TITLE_Y_OFFSET, .z = UI_Z_ORDER + 901,
		.type = GUI_ELEMENT_TEXT,
		.textId = GAME_TEXT_ID_RESULT_DEFEAT_TITLE,
		.textColor = PAL_COLOR_YELLOW,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.typed = { .text = { .maxX = MENU_BACK_X + MENU_BACK_WIDTH } }
	},
	{
		.x = MENU_BACK_X + RESULT_MESSAGE_X_OFFSET, .y = MENU_BACK_Y + 3 * MENU_TITLE_Y_OFFSET, .z = UI_Z_ORDER + 902,
		.type = GUI_ELEMENT_CUSTOM_TEXT,
		.textColor = PAL_COLOR_WHITE,
		.shadowTextColor = PAL_COLOR_BLACK,
		.textBackground = TRANSPARENT_INDEX,
		.typed = {
			.customText = {
				.text = get_lose_text,
				.maxHeight = MENU_BACK_HEIGHT  - 3 * MENU_TITLE_Y_OFFSET,
				.maxWidth = MENU_BACK_WIDTH - 2 * RESULT_MESSAGE_X_OFFSET
			}
		}
	},
	{
		.x = BUTTON_CONFIRM_RESULT_X, .y = BUTTON_CONFIRM_RESULT_Y, .z = UI_Z_ORDER + 902,
		.type = GUI_ELEMENT_BUTTON,
		.textId = GAME_TEXT_ID_RESULT_DEFEAT_ACCEPT,
		.textColor = PAL_COLOR_WHITE,
		.textBackground = TRANSPARENT_INDEX,
		.shadowTextColor = PAL_COLOR_BLACK,
		.hotkey = KEY_D,
		.typed = {
			.button = {
				.size = { .width = BUTTON_CONFIRM_RESULT_WIDTH, .height = BUTTON_HEIGHT },
				.action = go_results
			}
		}
	},
};

static GuiScreen guiScreenPlay = { .elements = playMenu, .elementsCount = PLAY_MENU_ELEMENTS };
static GuiScreen guiScreenWin = { .elements = winMenu, .elementsCount = WIN_MENU_ELEMENTS };
static GuiScreen guiScreenLose = { .elements = loseMenu, .elementsCount = LOSE_MENU_ELEMENTS };

static void handle_hover(GameContext *context) {
	showResourceTooltip = FALSE;
	MouseCursorStateEnum mouseState = game_mouse_get_cursor_state();
	if(mouseState != MOUSE_CURSOR_IDLE || !keyboard_is_key_down(KEY_TAB)) return;
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	// If we are not on the viewport, return
	if(mouseY < VIEWPORT_Y_MIN || mouseY > VIEWPORT_Y_MAX ||
		mouseX < VIEWPORT_X_MIN || mouseX > VIEWPORT_X_MAX) return;
	// If we hover on a resource, we show the name and the quantity of the resource in the tooltip
	int boardXPosition = game_spatial_get_board_x_position(context->xPosition, mouseX);
	int boardYPosition = game_spatial_get_board_y_position(context->yPosition, mouseY);
	if(context->boardExploration[boardXPosition][boardYPosition] == BOARD_UNEXPLORED) return;
	BoardTile* tile = &context->board[boardXPosition][boardYPosition];
	if (tile->type == TILE_TYPE_GOLD || tile->type == TILE_TYPE_WOOD) {
		itoa(tile->data, tooltipQtyBuffer, BASE_TEN_NUMBER);
		tooltipText = text_get_by_id(tile->type == TILE_TYPE_GOLD ? GAME_TEXT_ID_GOLD : GAME_TEXT_ID_WOOD);
		showResourceTooltip = TRUE;
		// Position tooltip above the mouse cursor, with some offset
		// Tooltip should not go outside screen bounds
		BITMAP * resourceHover = game_gfx_get_resource_hover();
		tooltipX = mouseX + resourceHover->w >= GAME_INTERNAL_WIDTH ? GAME_INTERNAL_WIDTH - resourceHover->w : mouseX;
		tooltipY = mouseY - resourceHover->h < 0 ? 0 : mouseY - resourceHover->h;
	}
}

static void render_tooltip(GameContext *context, RenderQueue *renderQueue) {
	if (!showResourceTooltip) return;
	BITMAP * resourceHover = game_gfx_get_resource_hover();
	render_queue_submit_solid(renderQueue, TOOLTIP_Z, resourceHover, tooltipX, tooltipY);
	render_queue_submit_text_shadow(renderQueue, TOOLTIP_Z + 1, context->gameFont, tooltipText,
		tooltipX + TOOLTIP_TEXT_X_OFFSET, tooltipY + TOOLTIP_TEXT_Y_OFFSET,
		PAL_COLOR_WHITE, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
	render_queue_submit_text_shadow(renderQueue, TOOLTIP_Z + 1, context->gameFont, tooltipQtyBuffer,
		tooltipX + TOOLTIP_TEXT_X_OFFSET, tooltipY + TOOLTIP_TEXT_Y_OFFSET + context->gameFont->height,
		PAL_COLOR_WHITE, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
}

static void game_update(GameContext *context) {

	// Inputs
	// Command bar has related inputs so must be first
	game_cmd_bar_handle_buttons(context);
	game_mouse_handle_status_change(context);
	game_gui_handle(context, &guiScreenPlay);
	handle_hover(context);
	if(context->targetBlinkTime) {
		context->targetBlinkTime--;
	}
	else {
		context->targetPosition.x = NO_TARGET_POSITION;
		context->targetPosition.y = NO_TARGET_POSITION;
	}
	
	if (keyboard_is_key_pressed(KEY_SPACE)) game_selection_center_camera_on_selection(context);
	// Resource debug keys
	#ifdef DEBUG
	if(keyboard_is_key_pressed(KEY_G)) context->isDebugEnabled ^= TRUE;
	if(context->isDebugEnabled) {
		if(keyboard_is_key_pressed(KEY_F11)) {
			for(int x = 0; x < BOARD_WIDTH; x++) {
				for(int y = 0; y < BOARD_HEIGHT; y++) {
					if(context->boardExploration[x][y] == BOARD_UNEXPLORED) {
						game_spatial_explore_position(context, x, y);
					}
				}
			}
		}
		if (keyboard_is_key_pressed(KEY_6)) {
			resource_add_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 1000);
		}
		if (keyboard_is_key_pressed(KEY_7)) {
			resource_add_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 1000);
		}
		if (keyboard_is_key_pressed(KEY_8)) {
			resource_deduct_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 1000);
		}
		if (keyboard_is_key_pressed(KEY_9)) {
			resource_deduct_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 1000);
		}
		if (keyboard_is_key_pressed(KEY_F12)) nextState = GAME_STATE_LOAD_MAP;
		if (keyboard_is_key_pressed(KEY_DEL)) {
			for (int i = 0; i < context->selectedUnitCount; i++) {
				GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
				if (unit && unit->isActive && unit->state != UNIT_STATE_DIE) {
					unit->health = 0;
					unit->state = UNIT_STATE_DIE;
					game_animation_unit_set(unit);
				}
			}
		}
	}
	#endif
	game_selection_handle_slots(context);

	// We are in building mode
	if(context->buildPlacing.showBuilding) {
		building_handle_placing_input(context);
	}
	else {
		game_selection_handle_input(context);
	}
	
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;

	// If we click the mouseStatus on the minimap, we should move the camera or attack/move
	if (mouseX >= MINIMAP_X_POS &&
		mouseX <= MINIMAP_X_POS + BOARD_WIDTH &&
		mouseY >= MINIMAP_Y_POS &&
		mouseY <= MINIMAP_Y_POS + BOARD_HEIGHT) {
		MouseCursorStateEnum mouseState = game_mouse_get_cursor_state();

		if (context->mouseStatus.isLeftDown && mouseState == MOUSE_CURSOR_IDLE) {
			context->xPosition = (mouseX - MINIMAP_X_POS - MINIMAP_CENTER_OFFSET_X) * TILE_SIZE;
			context->yPosition = (mouseY - MINIMAP_Y_POS - MINIMAP_CENTER_OFFSET_Y) * TILE_SIZE;
			if (context->yPosition < 0) context->yPosition = 0;
			if (context->xPosition < 0) context->xPosition = 0;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
		if ((mouseState == MOUSE_CURSOR_ATTACK || mouseState == MOUSE_CURSOR_TARGET) && context->mouseStatus.isLeftReleased) {
			uint16_t boardXPosition = clamp(mouseX - MINIMAP_X_POS, 0, BOARD_WIDTH);
			uint16_t boardYPosition = clamp(mouseY - MINIMAP_Y_POS, 0, BOARD_HEIGHT);
			for (int i = 0; i < context->selectedUnitCount; i++) {
				GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
				if (!unit) continue;
				if (mouseState == MOUSE_CURSOR_ATTACK) game_unit_command_move_attack_player(context, unit, NULL, boardXPosition, boardYPosition);
				if (mouseState == MOUSE_CURSOR_TARGET) game_unit_command_move_player(context, unit, NULL, boardXPosition, boardYPosition);
			}
			game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
		}
	}

	// Viewport moving
	if ((key[KEY_UP] || key[KEY_DOWN] || key[KEY_LEFT] || key[KEY_RIGHT]) ||
		((mouseX < MOUSE_X_GO_LEFT || mouseX > MOUSE_X_GO_RIGHT ||
		  mouseY < MOUSE_Y_GO_UP || mouseY > MOUSE_Y_GO_DOWN) &&
		 !context->mouseStatus.isLeftDown)) {
		moveViewportCounter++;
	} else {
		moveViewportCounter = 0;
	}
	int cameraSpeed = 4;

	if (moveViewportCounter >= 1) {
		if (key[KEY_UP] || mouseY < MOUSE_Y_GO_UP) {
			context->yPosition -= cameraSpeed;
			if (context->yPosition < 0) context->yPosition = 0;
		}
		if (key[KEY_DOWN] || mouseY > MOUSE_Y_GO_DOWN) {
			context->yPosition += cameraSpeed;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
		if (key[KEY_LEFT] || mouseX < MOUSE_X_GO_LEFT) {
			context->xPosition -= cameraSpeed;
			if (context->xPosition < 0) context->xPosition = 0;
		}
		if (key[KEY_RIGHT] || mouseX > MOUSE_X_GO_RIGHT) {
			context->xPosition += cameraSpeed;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
		}
		moveViewportCounter = 0;
	}

	// Logic
	game_unit_process_all(context);
	game_objects_advance(context);
	game_strategy_ai_execute(context);
	resource_update_ui_quantities(context);
	message_update();

	// Win condition
	// If active units are of only one controller, we go to load map again
	int playerUnitsCount = 0;
	for (int i = 0; i < context->activeUnitCount; i++) {
		GameUnit *unit = context->activeUnits[i];
		if (unit == NULL || unit->controller == UNIT_CONTROLLER_PLAYER) playerUnitsCount++;
	}
	if(playerUnitsCount == 0) context->gameResult = GAME_RESULT_DEFEAT;
	if (playerUnitsCount == context->activeUnitCount) context->gameResult = GAME_RESULT_VICTORY;
	// We can enter here via unit count or others
	if(context->gameResult == GAME_RESULT_DEFEAT || context->gameResult == GAME_RESULT_VICTORY) {
		menuBack = game_gfx_get_menu_back();
		game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	}
	player_attacked_update(&context->playerAttackedData);
}

static void minimap_render(GameContext *context, RenderQueue *renderQueue) {
	if(context->aiData.state != AI_STATE_CREATE_WORKERS) return;
	// Minimap update
	clear_bitmap(context->renderedMinimapUnits);
	GameUnit **activeUnits = context->activeUnits;
	for (int i = 0; i < context->activeUnitCount; i++, activeUnits++) {
		GameUnit *unit = *activeUnits;
		if(!game_unit_is_visible(context, unit)) continue;
		int color = unit->controller == UNIT_CONTROLLER_PLAYER ? PAL_COLOR_GREEN : PAL_COLOR_RED;
		if (unit->tileSize == 1) {
			putpixel(context->renderedMinimapUnits, unit->x, unit->y, color);
		} else {
			for (int dx = 0; dx < unit->tileSize; dx++) {
				for (int dy = 0; dy < unit->tileSize; dy++) {
					putpixel(context->renderedMinimapUnits, unit->x + dx, unit->y + dy, color);
				}
			}
		}
	}
}

void handle_play_map_render(GameContext *context, RenderQueue *renderQueue) {
	minimap_render(context, renderQueue);
	render_queue_add_active_units(context, renderQueue);
	render_queue_add_active_objects(context, renderQueue);
	render_queue_submit_ui(context, renderQueue);
	game_cmd_bar_render_queue_submit(context, renderQueue);
	message_render_queue_submit(renderQueue, context->gameFont);
	game_gui_render_queue_submit(context, renderQueue, &guiScreenPlay);
	render_tooltip(context, renderQueue);
	player_attacked_render(&context->playerAttackedData, renderQueue);
	switch(context->gameResult) {
		case GAME_RESULT_ONGOING: {
			// No result screen, just regular gameplay UI
			break;
		}
		case GAME_RESULT_VICTORY: {
			game_gui_render_queue_submit(context, renderQueue, &guiScreenWin);
			break;
		}
		case GAME_RESULT_DEFEAT: {
			game_gui_render_queue_submit(context, renderQueue, &guiScreenLose);
			break;
		}
	}
	if(!renderBackground) render_queue_submit_mouse(context, renderQueue);
	renderBackground = FALSE;
}

void handle_play_map_init(GameContext *context) {
	video_fade_in_init(DEFAULT_FADE_SPEED, context->mainPalette);
	renderBackground = FALSE;
	nextState = GAME_STATE_PLAY_MAP;
	game_strategy_ai_init(context);
}

GameStateEnum handle_play_map_update(GameContext *context) {
	switch(context->gameResult) {
		case GAME_RESULT_ONGOING: {
			game_update(context);
			break;
		}
		case GAME_RESULT_VICTORY: {
			game_gui_handle(context, &guiScreenWin);
			break;
		}
		case GAME_RESULT_DEFEAT: {
			game_gui_handle(context, &guiScreenLose);
			break;
		}
	}

	if(renderBackground) return GAME_STATE_PLAY_MAP;

	return nextState;
}

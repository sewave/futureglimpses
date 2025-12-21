#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

int moveViewportCounter = 0;



GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue) {
	// Inputs
	game_mouse_handle_status_change(context);

	// TODO menus
	if (keyboard_is_key_pressed(KEY_F12)) return GAME_STATE_EXIT;
	if (keyboard_is_key_pressed(KEY_F11)) return GAME_STATE_LOAD_MAP;
	if (keyboard_is_key_pressed(KEY_SPACE)) game_selection_center_camera_on_selection(context);
	// Resource debug keys
	if (keyboard_is_key_pressed(KEY_Y)) {
		resource_add_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 100);
	}
	if (keyboard_is_key_pressed(KEY_U)) {
		resource_add_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 100);
	}
	if (keyboard_is_key_pressed(KEY_J)) {
		resource_deduct_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_GOLD, 100);
	}
	if (keyboard_is_key_pressed(KEY_K)) {
		resource_deduct_amount(context, UNIT_CONTROLLER_PLAYER, RESOURCE_TYPE_WOOD, 100);
	}
	if (keyboard_is_key_pressed(KEY_L)) {
		context->config.lifeBar = (context->config.lifeBar + 1) % LIFE_BAR_COUNT;
	}

	game_selection_handle_slots(context);
	game_selection_handle_input(context);

	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;

	// If we click the mouseStatus on the minimap, we should move the camera or attack/move
	if (mouseX >= MINIMAP_X_POS &&
		mouseX <= MINIMAP_X_POS + BOARD_WIDTH &&
		mouseY >= MINIMAP_Y_POS &&
		mouseY <= MINIMAP_Y_POS + BOARD_HEIGHT) {
		MouseCursorStateEnum mouseState = game_mouse_get_cursor_state();

		if (context->mouseStatus.isLeftDown && mouseState == MOUSE_CURSOR_IDLE) {
			context->xPosition = (mouseX - MINIMAP_X_POS - 8) * TILE_SIZE;
			context->yPosition = (mouseY - MINIMAP_Y_POS - 6) * TILE_SIZE;
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
				if (mouseState == MOUSE_CURSOR_ATTACK) game_unit_command_move_attack(unit, NULL, boardXPosition, boardYPosition);
				if (mouseState == MOUSE_CURSOR_TARGET) game_unit_command_move(unit, NULL, boardXPosition, boardYPosition);
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

	game_unit_process_all(context);

	// Win condition
	// TODO WIN Message + WIN SCREEN
	// If active units are of only one controller, we go to load map again, TODO win/loss screen
	int playerUnitsCount = 0;
	for (int i = 0; i < context->activeUnitCount; i++) {
		GameUnit *unit = context->activeUnits[i];
		if (!unit || unit->controller == UNIT_CONTROLLER_PLAYER) playerUnitsCount++;
	}
	if (playerUnitsCount == context->activeUnitCount || playerUnitsCount == 0) return GAME_STATE_LOAD_MAP;

	// Logic
	game_objects_advance(context);
	game_strategy_ai_execute(context);
	resource_update_ui_quantities(context);

	// If there are more ticks to draw, skip queue phase, ups performance
	if (context->ticksToCatchup) return GAME_STATE_PLAY_MAP;

	// TODO should we have only one render function?
	render_queue_add_active_units(context, renderQueue);
	render_queue_add_active_objects(context, renderQueue);
	render_queue_submit_mouse(context, renderQueue);
	render_queue_submit_ui(context, renderQueue);

	return GAME_STATE_PLAY_MAP;
}

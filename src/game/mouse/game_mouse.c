#include "game_mouse.h"

static MouseCursorStateEnum mouseCursorState = MOUSE_CURSOR_IDLE;

static const char *mouseCursorFilenames[NUM_MOUSE_CURSORS] = {
		"assets/mouse/idle.pcx",
		"assets/mouse/look.pcx",
		"assets/mouse/none.pcx",
		"assets/mouse/target.pcx",
		"assets/mouse/attack.pcx",
		"assets/mouse/none.pcx",
};

static const int8_t cursorFocus[NUM_MOUSE_CURSORS][2] = {
	{0, 0}, {8, 8}, {0, 0}, {8, 8}, {8, 8}, {0, 0},
};

static void game_mouse_go_attack_or_move(GameContext* context) {
	if(context->selectedUnitCount > 0) {
		if (keyboard_is_key_pressed(KEY_A)) game_mouse_set_cursor_state(MOUSE_CURSOR_ATTACK);
		if (keyboard_is_key_pressed(KEY_M)) game_mouse_set_cursor_state(MOUSE_CURSOR_TARGET);
	}
}

InitializationStatusEnum game_mouse_init_cursors() {
	if (mouse_init_cursors(NUM_MOUSE_CURSORS, mouseCursorFilenames) != INITIALIZATION_OK) return INITIALIZATION_ERROR;
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
	return INITIALIZATION_OK;
}

void game_mouse_set_cursor_state(MouseCursorStateEnum state) {
	mouseCursorState = state;
	set_mouse_sprite_focus(cursorFocus[mouseCursorState][0], cursorFocus[mouseCursorState][1]);
	mouse_set_cursor(mouseCursorState);
}

MouseCursorStateEnum game_mouse_get_cursor_state() {
	return mouseCursorState;
}

void game_mouse_handle_status_change(GameContext *context) {
	GameUnit *sourceUnit = NULL;

	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;

	// If we are on the viewport, check for targets
	if (mouseY > VIEWPORT_Y_MIN && mouseY < VIEWPORT_Y_MAX &&
		mouseX > VIEWPORT_X_MIN && mouseX < VIEWPORT_X_MAX) {
		int tileX = game_spatial_get_board_x_position(context->xPosition, mouseX);
		int tileY = game_spatial_get_board_y_position(context->yPosition, mouseY);
		UnitId id = game_selection_get_in_position_or_previous(context, tileX, tileY);
		sourceUnit = game_unit_get_by_id(context, id);
	}

	// TODO: based on active command buttons
	switch (mouseCursorState) {
		case MOUSE_CURSOR_IDLE:
			if (sourceUnit && sourceUnit->controller == UNIT_CONTROLLER_PLAYER) {
				game_mouse_set_cursor_state(MOUSE_CURSOR_LOOK);
			}
			game_mouse_go_attack_or_move(context);
			break;
		case MOUSE_CURSOR_LOOK:
			if (!sourceUnit) game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
			game_mouse_go_attack_or_move(context);
			break;
		case MOUSE_CURSOR_ATTACK:
		case MOUSE_CURSOR_TARGET:
			// If we cancel or have no selected units
			if (keyboard_is_key_pressed(KEY_ESC) || context->selectedUnitCount == 0) {
				game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
			}
			break;
		default:
			// Nothin else
			break;
	}
}

void game_mouse_queue_draw(GameContext *context, RenderQueue *renderQueue, int selectionStartX, int selectionStartY) {
	switch (mouseCursorState) {
		case MOUSE_CURSOR_SELECT:
			// Selection rectangle
			int selectionEndX = clamp(context->mouseStatus.x, VIEWPORT_X_MIN, VIEWPORT_X_MAX);
			int selectionEndY = clamp(context->mouseStatus.y, VIEWPORT_Y_MIN, VIEWPORT_Y_MAX);
			render_queue_submit_rect(renderQueue,
									 MOUSE_Z_ORDER,
									 selectionStartX, selectionStartY, selectionEndX, selectionEndY,
									 PAL_COLOR_GREEN);
			break;
		default:
			// Mouse cursor
			render_queue_submit_sprite(renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(),
									   context->mouseStatus.x - mouse_x_focus, context->mouseStatus.y - mouse_y_focus,
									   RND_FLAG_NORMAL);
			break;
	}
}

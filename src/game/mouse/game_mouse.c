#include <allegro/gfx.h>
#include "game/mouse/game_mouse.h"
#include "game/video/render.h"

static MouseCursorStateEnum mouseCursorState = MOUSE_CURSOR_IDLE;
BITMAP *arrowCursors;

static int selectionStartX, selectionStartY;

static const char *mouseCursorFilenames[NUM_MOUSE_CURSORS] = {
		"assets/mouse/idle.pcx",
		"assets/mouse/look.pcx",
		"assets/mouse/none.pcx",
		"assets/mouse/target.pcx",
		"assets/mouse/attack.pcx",
		"assets/mouse/none.pcx",
};

static const int8_t cursorFocus[NUM_MOUSE_CURSORS][2] = {
		{0, 0},
		{8, 8},
		{0, 0},
		{8, 8},
		{8, 8},
		{0, 0},
};

#define ARROWS_FILENAME "assets/mouse/arrows.pcx"

InitializationStatusEnum game_mouse_init_cursors() {
	if (mouse_init_cursors(NUM_MOUSE_CURSORS, mouseCursorFilenames) != INITIALIZATION_OK) return INITIALIZATION_ERROR;
	arrowCursors = load_bitmap(ARROWS_FILENAME, NULL);
	if (arrowCursors == NULL) return INITIALIZATION_ERROR;
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

	switch (mouseCursorState) {
		case MOUSE_CURSOR_IDLE:
			if (sourceUnit && sourceUnit->controller == UNIT_CONTROLLER_PLAYER) {
				game_mouse_set_cursor_state(MOUSE_CURSOR_LOOK);
			}
			break;
		case MOUSE_CURSOR_LOOK:
			if (!sourceUnit) game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
			break;
		case MOUSE_CURSOR_ATTACK:
		case MOUSE_CURSOR_TARGET:
			if (context->selectedUnitCount == 0) game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
			break;
		default:
			// Nothin else
			break;
	}
}

void game_mouse_destroy_cursors() {
	mouse_destroy_cursors();
	if (arrowCursors) destroy_bitmap(arrowCursors);
}

void game_mouse_start_selection(int x, int y) {
	selectionStartX = x;
	selectionStartY = y;
}

int game_mouse_get_selection_start_x() {
	return selectionStartX;
}

int game_mouse_get_selection_start_y() {
	return selectionStartY;
}

BITMAP *game_mouse_get_arrow_cursors_sheet() {
	return arrowCursors;
}

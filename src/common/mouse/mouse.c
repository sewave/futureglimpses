#include <stdlib.h>
#include <stdio.h>
#include <allegro/file.h>
#include <allegro/datafile.h>
#include "common/mouse.h"
#include "common/console.h"

#define DOUBLE_CLICK_OFFSET_PIXELS 4

static BITMAP **mouseCursors = NULL;
static uint8_t numMouseCursors = 0;
static uint8_t currentMouseCursor = 0;

static int mouse_get_x() {
#ifdef DOS
	return mouse_x;
#else
	return mouse_x / X_RESOLUTION_MULTIPLIER;
#endif
}

static int mouse_get_y() {
#ifdef DOS
	return mouse_y;
#else
	return mouse_y / Y_RESOLUTION_MULTIPLIER;
#endif
}

static void mouse_load_cursor(uint8_t index, BITMAP *cursor) {
	if (index >= numMouseCursors) return;
	if (mouseCursors[index] != NULL) destroy_bitmap(mouseCursors[index]);
	mouseCursors[index] = cursor;
}

InitializationStatusEnum mouse_init_cursors(uint8_t numCursors, const char **mouseCursorFilenames) {
	ConsoleCoords cursorPos = console_get_cursor_position();
	common_print_load_step(cursorPos.x, cursorPos.y);
	if (install_mouse() < ALLEGRO_INIT_OK) return INITIALIZATION_ERROR;
	show_mouse(NULL);
	numMouseCursors = numCursors;
	mouseCursors = (BITMAP **) calloc(numMouseCursors, sizeof(BITMAP *));
	common_print_load_step(cursorPos.x, cursorPos.y);
	for (int i = 0; i < numMouseCursors; i++) {
		BITMAP *cursorBitmap = load_bitmap(mouseCursorFilenames[i], NULL);
		if (cursorBitmap == NULL) return INITIALIZATION_ERROR;
		mouse_load_cursor(i, cursorBitmap);
		common_print_load_step(cursorPos.x, cursorPos.y);
	}
	return INITIALIZATION_OK;
}

BITMAP *mouse_get_cursor_sprite() {
	return mouseCursors[currentMouseCursor];
}

void mouse_set_cursor(uint8_t index) {
	currentMouseCursor = index;
}

void mouse_destroy_cursors(void) {
	for (int i = 0; i < numMouseCursors; i++) {
		if (mouseCursors[i] != NULL) {
			destroy_bitmap(mouseCursors[i]);
			mouseCursors[i] = NULL;
		}
	}
	free(mouseCursors);
	mouseCursors = NULL;
}

void mouse_initialize_status(MouseStatus *status, uint32_t doubleClickMaxTime) {
	status->x = mouse_get_x();
	status->y = mouse_get_y();
	status->clickX = 0;
	status->clickY = 0;
	status->isLeftDown = FALSE;
	status->isRightDown = FALSE;
	status->wasLeftDown = FALSE;
	status->wasRightDown = FALSE;
	status->isLeftPressed = FALSE;
	status->isRightPressed = FALSE;
	status->isLeftReleased = FALSE;
	status->isRightReleased = FALSE;
	status->isLeftDoubleClick = FALSE;
	status->leftClickTime = 0;
	status->leftClickMaxTime = doubleClickMaxTime;
	mouse_update_status(status);
}

void mouse_update_status(MouseStatus *status) {
	poll_mouse();
	int x = mouse_get_x();
	int y = mouse_get_y();

	status->x = x;
	status->y = y;

	int leftButton = mouse_b & 1;
	int rightButton = (mouse_b & 2) >> 1;

	status->isLeftPressed = (!status->isLeftDown) && leftButton;
	if(status->isLeftPressed) {
		if(abs(status->clickX - status->x) + abs(status->clickY - status->y) > DOUBLE_CLICK_OFFSET_PIXELS) {
			status->leftClickTime = 0;
		}
		status->clickX = status->x;
		status->clickY = status->y;
	}
	status->isRightPressed = (!status->isRightDown) && rightButton;
	status->isLeftReleased = status->isLeftDown && (!leftButton);
	status->isRightReleased = status->isRightDown && (!rightButton);

	status->wasLeftDown = status->isLeftDown;
	status->wasRightDown = status->isRightDown;

	status->isLeftDown = leftButton;
	status->isRightDown = rightButton;

	// Handle double click
	status->isLeftDoubleClick = FALSE;
	if (status->isLeftReleased) {
		if (status->leftClickTime > 0 && status->leftClickTime < status->leftClickMaxTime) {
			status->isLeftDoubleClick = TRUE;
			status->leftClickTime = 0;
		} else {
			status->isLeftDoubleClick = FALSE;
			status->leftClickTime = 1;
		}
	} else {
		if (status->leftClickTime > 0) {
			status->leftClickTime++;
			if (status->leftClickTime >= status->leftClickMaxTime) status->leftClickTime = 0;
		}
	}
}

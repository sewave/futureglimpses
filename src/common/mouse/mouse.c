#include <allegro/gfx.h>
#include <allegro/file.h>
#include <allegro/datafile.h>
#include "mouse.h"
#include <stdlib.h>

static BITMAP **mouseCursors = NULL;
static uchar numMouseCursors = 0;
static uchar currentMouseCursor = 0;

static int mouse_get_x() {
    #ifdef DOS
        return mouse_x;
    #else
        return mouse_x / RESOLUTION_MULTIPLIER;
    #endif
}

static int mouse_get_y() {
    #ifdef DOS
        return mouse_y;
    #else
        return mouse_y / RESOLUTION_MULTIPLIER;
    #endif
}

static void mouse_load_cursor(uchar index, BITMAP* cursor) {
    if (index >= numMouseCursors) return;
    if (mouseCursors[index] != NULL) destroy_bitmap(mouseCursors[index]);
    mouseCursors[index] = cursor;
}

InitializationStatusEnum mouse_init_cursors(uchar numCursors, const char** mouseCursorFilenames) {
    if (install_mouse() < ALLEGRO_INIT_OK) return INITIALIZATION_ERROR;
    show_mouse(NULL);
	numMouseCursors = numCursors;
	mouseCursors = (BITMAP **) calloc(numMouseCursors, sizeof(BITMAP *));
	for (int i = 0; i < numMouseCursors; i++) {
        BITMAP* cursorBitmap = load_bitmap(mouseCursorFilenames[i], NULL);
        if (cursorBitmap == NULL) return INITIALIZATION_ERROR;
        mouse_load_cursor(i, cursorBitmap);
    }
    return INITIALIZATION_OK;
}

BITMAP *mouse_get_cursor_sprite() {
	return mouseCursors[currentMouseCursor];
}

void mouse_set_cursor(uchar index) {
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
}

void mouse_initialize_status(MouseStatus* status) {
    status->x = mouse_get_x();
    status->y = mouse_get_y();
    status->isLeftDown = FALSE;
    status->isRightDown = FALSE;
    status->wasLeftDown = FALSE;
    status->wasRightDown = FALSE;
    status->isLeftPressed = FALSE;
    status->isRightPressed = FALSE;
    status->isLeftReleased = FALSE;
    status->isRightReleased = FALSE;
    status->isSelecting = FALSE;
    mouse_update_status(status);
}

void mouse_update_status(MouseStatus* status) {
    //if(!mouse_needs_poll()) return;
    poll_mouse();
    int x = mouse_get_x();
    int y = mouse_get_y();
    status->x = x;
    status->y = y;

    int leftButton = mouse_b & 1;
    int rightButton = (mouse_b & 2) >> 1;

    status->isLeftPressed = (!status->isLeftDown) && leftButton;
    status->isRightPressed = (!status->isRightDown) && rightButton;
    status->isLeftReleased = status->isLeftDown && (!leftButton);
    status->isRightReleased = status->isRightDown && (!rightButton);

    status->wasLeftDown = status->isLeftDown;
    status->wasRightDown = status->isRightDown;

    status->isLeftDown = leftButton;
    status->isRightDown = rightButton;
}

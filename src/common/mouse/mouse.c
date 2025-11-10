#include <allegro/gfx.h>
#include <allegro/file.h>
#include <allegro/datafile.h>
#include "mouse.h"
#include <stdlib.h>

BITMAP **mouseCursors = NULL;
uchar numMouseCursors = 0;
uchar currentMouseCursor = 0;

void _mouse_load_cursor(uchar index, BITMAP* cursor) {
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
        _mouse_load_cursor(i, cursorBitmap);
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

int mouse_get_x() {
    #ifdef DOS
        return mouse_x;
    #else
        return mouse_x / RESOLUTION_MULTIPLIER;
    #endif
}

int mouse_get_y() {
    #ifdef DOS
        return mouse_y;
    #else
        return mouse_y / RESOLUTION_MULTIPLIER;
    #endif
}

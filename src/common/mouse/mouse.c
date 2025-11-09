#include <allegro/gfx.h>
#include "mouse.h"
#include <stdlib.h>

BITMAP **mouseCursors = NULL;
uchar numMouseCursors = 0;
uchar currentMouseCursor = 0;

InitializationStatusEnum mouse_init_cursors(uchar numCursors) {
    if (install_mouse() < ALLEGRO_INIT_OK) return INITIALIZATION_ERROR;
    show_mouse(NULL);
	numMouseCursors = numCursors;
	mouseCursors = (BITMAP **) calloc(numMouseCursors, sizeof(BITMAP *));
	for (int i = 0; i < numMouseCursors; i++) mouseCursors[i] = NULL;
    return INITIALIZATION_OK;
}

BITMAP *mouse_get_cursor() {
	return mouseCursors[currentMouseCursor];
}

void mouse_set_cursor(uchar index) {
	currentMouseCursor = index;
}

void mouse_load_cursor(uchar index, BITMAP* cursor) {
    if (index >= numMouseCursors) return;
    if (mouseCursors[index] != NULL) destroy_bitmap(mouseCursors[index]);
    mouseCursors[index] = cursor;
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

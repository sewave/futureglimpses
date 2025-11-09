#include "game_mouse.h"
#include <allegro/gfx.h>
#include <allegro/file.h>
#include <allegro/datafile.h>

MouseCursorStateEnum mouseCursorState = MOUSE_CURSOR_IDLE;

InitializationStatusEnum game_mouse_init_cursors() {
    if(mouse_init_cursors(NUM_MOUSE_CURSOR_STATES) != INITIALIZATION_OK) return INITIALIZATION_ERROR;
    mouse_load_cursor(MOUSE_CURSOR_IDLE, load_bitmap("assets/mouse/idle.pcx", NULL));
    mouse_load_cursor(MOUSE_CURSOR_ATTACK, load_bitmap("assets/mouse/idle.pcx", NULL));
    mouse_load_cursor(MOUSE_CURSOR_CLICK, load_bitmap("assets/mouse/idle.pcx", NULL));
    mouse_load_cursor(MOUSE_CURSOR_SELECT, load_bitmap("assets/mouse/idle.pcx", NULL));
    mouse_load_cursor(MOUSE_CURSOR_TARGET, load_bitmap("assets/mouse/idle.pcx", NULL));
    mouseCursorState = MOUSE_CURSOR_IDLE;
    return INITIALIZATION_OK;
}

void game_mouse_set_cursor_state(MouseCursorStateEnum state) {
    mouse_set_cursor(state);
    mouseCursorState = state;
}

MouseCursorStateEnum game_mouse_get_cursor_state() {
    return mouseCursorState;
}

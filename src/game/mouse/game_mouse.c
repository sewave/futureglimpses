#include "game_mouse.h"

MouseCursorStateEnum mouseCursorState = MOUSE_CURSOR_IDLE;

const char* mouseCursorFilenames [NUM_MOUSE_CURSORS] = {
    "assets/mouse/idle.pcx",
    "assets/mouse/idle.pcx",
    "assets/mouse/idle.pcx",
    "assets/mouse/idle.pcx",
    "assets/mouse/idle.pcx"
};

InitializationStatusEnum game_mouse_init_cursors() {
    if(mouse_init_cursors(NUM_MOUSE_CURSORS, mouseCursorFilenames) != INITIALIZATION_OK) return INITIALIZATION_ERROR;
    game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
    return INITIALIZATION_OK;
}

void game_mouse_set_cursor_state(MouseCursorStateEnum state) {
    mouseCursorState = state;
    mouse_set_cursor(mouseCursorState);
}

MouseCursorStateEnum game_mouse_get_cursor_state() {
    return mouseCursorState;
}

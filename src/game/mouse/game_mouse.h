#ifndef GAME_MOUSE_H
#define GAME_MOUSE_H
#include "../game_lib.h"

typedef enum {
	MOUSE_CURSOR_IDLE,
	MOUSE_CURSOR_CLICK,
	MOUSE_CURSOR_SELECT,
	MOUSE_CURSOR_ATTACK,
	MOUSE_CURSOR_TARGET,
	NUM_MOUSE_CURSORS
} MouseCursorStateEnum;

InitializationStatusEnum game_mouse_init_cursors();
void game_mouse_set_cursor_state(MouseCursorStateEnum state);
MouseCursorStateEnum game_mouse_get_cursor_state();

#endif

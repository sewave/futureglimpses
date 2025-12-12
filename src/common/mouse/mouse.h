#ifndef MOUSE_H
#define MOUSE_H
#include "../../common/common.h"
#include <allegro/gfx.h>
#include <allegro/mouse.h>

typedef struct {
    int x, y;
    uint8_t isLeftDown;
    uint8_t isRightDown;
    uint8_t wasLeftDown;
    uint8_t wasRightDown;
    uint8_t isLeftPressed;
    uint8_t isRightPressed;
    uint8_t isLeftReleased;
    uint8_t isRightReleased;
    uint8_t isSelecting;
} MouseStatus;

BITMAP *mouse_get_cursor_sprite();
void mouse_set_cursor(uchar index);
void mouse_destroy_cursors(void);
InitializationStatusEnum mouse_init_cursors(uchar numCursors, const char** mouseCursorFilenames);
void mouse_initialize_status(MouseStatus* status);
void mouse_update_status(MouseStatus* status);

#endif

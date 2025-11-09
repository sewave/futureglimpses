#ifndef MOUSE_H
#define MOUSE_H
#include "../../common/common.h"
#include <allegro/gfx.h>
#include <allegro/mouse.h>

BITMAP *mouse_get_cursor();
void mouse_set_cursor(uchar index);
void mouse_destroy_cursors(void);
InitializationStatusEnum mouse_init_cursors(uchar numCursors, const char** mouseCursorFilenames);

#endif

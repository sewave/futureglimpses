#ifndef MOUSE_H
#define MOUSE_H
#include "../../common/common.h"
#include <allegro/gfx.h>
#include <allegro/mouse.h>

extern BITMAP* mouse_get_cursor();
extern void mouse_set_cursor(uchar index);
extern void mouse_load_cursor(uchar index, BITMAP* cursor);
extern void mouse_destroy_cursors(void);
extern InitializationStatusEnum mouse_init_cursors(uchar numCursors);

#endif

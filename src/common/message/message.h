#ifndef MESSAGE_H
#define MESSAGE_H
#include "../common_lib.h"

void message_init(int pMessageX, int pMessageStartY, int pMessageYInc, int pMessageZ);
void message_reset();
void message_add_to_queue(const char *message, uint16_t showTime, int color, int background);
void message_add_to_queue_shadow(const char *message, uint16_t showTime, int color, int background, int shadowColor);
void message_update();
void message_render_queue_submit(RenderQueue* renderQueue, FONT* font);
#endif /* MESSAGE_H */

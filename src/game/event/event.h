#ifndef EVENT_H
#define EVENT_H
#include "../game_lib.h"

void game_event_unit_process(GameContext *context, EventType eventType, GameUnit *unit, uint16_t data);
void game_event_object_process(GameContext *context, EventType eventType, Object *object, uint16_t data);

#endif /* EVENT_H */

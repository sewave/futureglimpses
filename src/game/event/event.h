#ifndef GAME_EVENT_H
#define GAME_EVENT_H
#include "../game_lib.h"

void game_event_unit_process(GameContext *context, EventType eventType, GameUnit *unit, uint16_t data);
void game_event_object_process(GameContext *context, EventType eventType, Object *object, uint16_t data);

#endif /* GAME_EVENT_H */

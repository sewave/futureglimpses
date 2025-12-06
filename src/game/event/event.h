#ifndef GAME_EVENT_H
#define GAME_EVENT_H
#include "../game_lib.h"

void game_event_process(GameContext* context, EventType eventType, GameUnit* unit, uint16_t data);

#endif /* GAME_EVENT_H */

#include "event.h"

void game_event_process(GameContext* context, EventType eventType, GameUnit* unit, uint16_t data) {
    switch(eventType) {
        case EVENT_TYPE_DAMAGE:
            GameUnit* target = game_unit_get_by_id(context, unit->targetId);
            if(target && target->isActive) game_unit_damage(context, unit, target);
        break;
        case EVENT_TYPE_SOUND:
            game_snd_play_sound((GameSound) data);
        break;
    }
}

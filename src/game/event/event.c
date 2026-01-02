#include "event.h"

void game_event_unit_process(GameContext *context, EventType eventType, GameUnit *unit, uint16_t data) {
	switch (eventType) {
		case EVENT_TYPE_DAMAGE:
			GameUnit *damageTarget = game_unit_get_by_id(context, unit->targetId);
			if (damageTarget) game_unit_damage(context, unit->minDamage, unit->maxDamage, damageTarget);
			break;
		case EVENT_TYPE_SOUND:
			if (unit->x >= context->xPosition / TILE_SIZE && unit->x <= (context->xPosition + VIEWPORT_WIDTH) / TILE_SIZE &&
				unit->y >= context->yPosition / TILE_SIZE && unit->y <= (context->yPosition + VIEWPORT_WIDTH) / TILE_SIZE) {
				game_snd_play_sound((GameSound) data);
			}
			break;
		case EVENT_TYPE_SPAWN_ARROW:
			GameUnit *arrowTarget = game_unit_get_by_id(context, unit->targetId);
			if (arrowTarget) game_object_spawn(context, OBJ_TYPE_ARROW, unit->controller, unit->x, unit->y, unit, arrowTarget, NO_TARGET_POSITION, NO_TARGET_POSITION);
			break;
		case EVENT_TYPE_SPAWN_FIREBALL:
			GameUnit *fireballTarget = game_unit_get_by_id(context, unit->targetId);
			if (fireballTarget) game_object_spawn(context, OBJ_TYPE_FIREBALL, unit->controller, unit->x, unit->y, unit, fireballTarget, NO_TARGET_POSITION, NO_TARGET_POSITION);
			break;
		case EVENT_TYPE_WORK:
			game_unit_work(context, unit);
			break;
		case EVENT_TYPE_AREA_DAMAGE:
			// Not used by units
		break;
	}
}

void game_event_object_process(GameContext *context, EventType eventType, Object *object, uint16_t data) {
	switch (eventType) {
		case EVENT_TYPE_DAMAGE:
			GameUnit *damageTarget = game_unit_get_by_id(context, object->targetId);
			if (damageTarget) {
				game_unit_damage(context, object->minDamage, object->maxDamage, damageTarget);
				if(damageTarget->state == UNIT_STATE_IDLE) {
					game_unit_command_move_attack(damageTarget, NULL, object->x / TILE_SIZE, object->y / TILE_SIZE);
				}
			}
			break;
		case EVENT_TYPE_AREA_DAMAGE:
			game_unit_area_damage(context, object);
			break;
		case EVENT_TYPE_SOUND:
			if (object->x >= context->xPosition && object->x <= (context->xPosition + VIEWPORT_WIDTH) &&
				object->y >= context->yPosition && object->y <= (context->yPosition + VIEWPORT_WIDTH)) {
				game_snd_play_sound((GameSound) data);
			}
			break;
		case EVENT_TYPE_SPAWN_ARROW:
		case EVENT_TYPE_SPAWN_FIREBALL:
		case EVENT_TYPE_WORK:
			// Do nothing
			break;
	}
}

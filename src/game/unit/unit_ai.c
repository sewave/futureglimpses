#include "unit_ai.h"

#define MAX_FOUND_UNITS 32
static GameUnit* foundUnits[MAX_FOUND_UNITS];

static void game_unit_ai_idle(GameContext *context, GameUnit *unit) {
	if (++unit->reactionTimeCounter >= unit->reactionTime) {
		unit->reactionTimeCounter = 0;
		uint16_t foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->maxAttackRange,
												  game_spatial_filter_enemy_units, unit, foundUnits,
												  1);
		// TODO Attack by priority?
		if (foundUnitsCount > 0) {
			for(int i = 0; i < foundUnitsCount; i++) {
				GameUnit *target = foundUnits[i];
				if (!game_spatial_unit_in_range(unit, target, unit->minAttackRange)) {
					game_unit_command_attack(unit, target, UNIT_STATE_IDLE);
					return;
				}
			}
			// TODO flee if we are ranged and target is too close?
		} else {
			foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->sightRange,
														game_spatial_filter_enemy_units, unit, foundUnits,
														MAX_FOUND_UNITS);
			if (foundUnitsCount > 0) {
				// Target nearest unit
				GameUnit *target = NULL;
				int distance = 9999999;
				GameUnit **newTargetList = foundUnits;
				for(int i = 0; i < foundUnitsCount; i++, newTargetList++) {
					GameUnit *newTarget = *newTargetList;
					int newDistance = distance_sq(unit->x, unit->y, newTarget->x, newTarget->y);
					if(newDistance < distance) {
						target = newTarget;
						distance = newDistance;
					}
				}
				if (target) game_unit_command_move_attack(unit, target, NO_TARGET_POSITION, NO_TARGET_POSITION);
			}
			else {
				// We found nothing, so we change direction to make it "look" around
				unit->direction = (unit->direction + 1) % DIRECTIONS_COUNT; 
				game_animation_unit_set(unit);
			}
		}
	}
}

static void game_unit_ai_move(GameContext *context, GameUnit *unit) {
	uint16_t targetX, targetY;
	if (unit->targetX != NO_TARGET_POSITION && unit->targetY != NO_TARGET_POSITION) {
		targetX = unit->targetX;
		targetY = unit->targetY;
	} else {
		GameUnit *targetUnit = game_unit_get_by_id(context, unit->targetId);
		if (!targetUnit) {
			game_unit_command_idle(unit);
			return;
		}
		targetX = targetUnit->x;
		targetY = targetUnit->y;
	}

	if (unit->x == targetX && unit->y == targetY) {
		game_unit_command_idle(unit);
		return;
	}

	if (context->walkabilityGrid[targetX][targetY] != WALKABILITY_FREE && game_spatial_target_in_range(unit, targetX, targetY, 1)) {
		game_unit_command_idle(unit);
		return;
	}

	if (game_unit_path_find(context, unit, targetX, targetY)) {
		game_unit_command_set_move_anim(unit, UNIT_STATE_MOVE);
	}
}

static void game_unit_ai_defend(GameContext *context, GameUnit *unit) {
	if (++unit->reactionTimeCounter >= unit->reactionTime) {
		unit->reactionTimeCounter = 0;
		uint16_t foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->maxAttackRange,
												  game_spatial_filter_enemy_units, unit, foundUnits,
												  1);
		if (foundUnitsCount > 0) game_unit_command_attack(unit, foundUnits[0], UNIT_STATE_DEFEND);
	}
}

static void game_unit_ai_attack(GameContext *context, GameUnit *unit) {
	GameUnit *target = game_unit_get_by_id(context, unit->targetId);
	if (game_animation_finished(&unit->animationStatus) || !target || !target->isActive || target->state == UNIT_STATE_DIE) {
		if (target && game_spatial_unit_in_range(unit, target, unit->maxAttackRange)
	&& !game_spatial_unit_in_range(unit, target, unit->minAttackRange)) {
			game_unit_face_target(unit, target);
			game_animation_reset(&unit->animationStatus);
		} else {
			unit->state = unit->nextState;
			unit->nextState = UNIT_STATE_IDLE;
			unit->targetX = unit->prevTargetX;
			unit->targetY = unit->prevTargetY;
			unit->targetId = unit->prevTargetId;
			game_animation_unit_set(unit);
		}
	}
}

static void game_unit_ai_move_anim(GameContext *context, GameUnit *unit) {
	if (++unit->moveTimeCounter > unit->moveTimeAnim) {
		unit->prevX = unit->x;
		unit->prevY = unit->y;
		unit->state = unit->nextState;
		unit->nextState = UNIT_STATE_IDLE;
		game_animation_unit_set(unit);
	}
}

static void game_unit_ai_move_attack(GameContext *context, GameUnit *unit) {
	uint16_t targetX, targetY;
	GameUnit *targetUnit = NULL;
	if (unit->targetX != NO_TARGET_POSITION && unit->targetY != NO_TARGET_POSITION) {
		targetX = unit->targetX;
		targetY = unit->targetY;
	} else {
		targetUnit = game_unit_get_by_id(context, unit->targetId);
		if (!targetUnit) {
			game_unit_command_idle(unit);
			return;
		}
		targetX = targetUnit->x;
		targetY = targetUnit->y;
	}

	if (unit->x == targetX && unit->y == targetY) {
		game_unit_command_idle(unit);
		return;
	}

	if (context->walkabilityGrid[targetX][targetY] != WALKABILITY_FREE &&
			game_spatial_target_in_range(unit, targetX, targetY, unit->maxAttackRange)
		&& !game_spatial_target_in_range(unit, targetX, targetY, unit->minAttackRange)) {
		if(targetUnit) {
			game_unit_command_attack(unit, targetUnit, UNIT_STATE_IDLE);
		}
		else {
			game_unit_command_idle(unit);
		}
		return;
	}

	uint16_t foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->maxAttackRange,
												  game_spatial_filter_enemy_units, unit, foundUnits,
												  1);
	if (foundUnitsCount > 0) {
		for(int i = 0; i < foundUnitsCount; i++) {
			GameUnit *target = foundUnits[i];
			if (!game_spatial_unit_in_range(unit, target, unit->minAttackRange)) {
				unit->prevTargetX = targetX;
				unit->prevTargetY = targetY;
				unit->prevTargetId = unit->targetId;
				game_unit_command_attack(unit, target, UNIT_STATE_MOVE_ATTACK);
				break;
			}
		}
		// TODO flee if we are ranged and target is too close?
	}
	else {
		if (game_unit_path_find(context, unit, targetX, targetY)) {
			game_unit_command_set_move_anim(unit, UNIT_STATE_MOVE_ATTACK);
		}
	}
}

static void game_unit_ai_die(GameContext *context, GameUnit *unit) {
	if (game_animation_finished(&unit->animationStatus)) game_unit_destroy(context, unit->id);
}

void game_unit_ai_invoke(GameContext *context, GameUnit *unit) {
	if(unit->isBuilding) return;
	switch (unit->state) {
		case UNIT_STATE_IDLE:
			game_unit_ai_idle(context, unit);
			break;
		case UNIT_STATE_ATTACK:
			game_unit_ai_attack(context, unit);
			break;
		case UNIT_STATE_DEFEND:
			game_unit_ai_defend(context, unit);
			break;
		case UNIT_STATE_MOVE:
			game_unit_ai_move(context, unit);
			break;
		case UNIT_STATE_MOVE_ANIM:
			game_unit_ai_move_anim(context, unit);
			break;
		case UNIT_STATE_MOVE_ATTACK:
			game_unit_ai_move_attack(context, unit);
			break;
        case UNIT_STATE_WORK:
            // TODO
	        break;
		case UNIT_STATE_DIE:
			game_unit_ai_die(context, unit);
			break;
		case UNIT_STATES_COUNT:
			// Nothing, to disable warning
			break;
	}
}

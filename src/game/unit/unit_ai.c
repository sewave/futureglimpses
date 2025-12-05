#include "unit_ai.h"

#define MAX_FOUND_UNITS 32
static UnitId foundUnitIds[MAX_FOUND_UNITS];
static uint16_t foundUnitsCount;

static void game_unit_ai_idle(GameContext *context, GameUnit *unit) {
	if (++unit->reactionTimeCounter >= unit->reactionTime) {
		unit->reactionTimeCounter = 0;
		foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->attackRange,
												  game_spatial_filter_enemy_units, unit, foundUnitIds,
												  MAX_FOUND_UNITS);
		if (foundUnitsCount > 0) {
			GameUnit *target = game_unit_get_by_id(context, foundUnitIds[0]);
			if (target) {
				game_unit_command_attack(unit, target, UNIT_STATE_IDLE);
			} else {
				foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->sightRange,
														  game_spatial_filter_enemy_units, unit, foundUnitIds,
														  MAX_FOUND_UNITS);
				if (foundUnitsCount > 0) {
					GameUnit *target = game_unit_get_by_id(context, foundUnitIds[0]);
					if (target) {
						game_unit_command_move_attack(unit, target, NO_TARGET_POSITION, NO_TARGET_POSITION);
					}
				}
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
		game_unit_command_move_anim(unit, UNIT_STATE_MOVE);
	}
}

static void game_unit_ai_defend(GameContext *context, GameUnit *unit) {
	if (++unit->reactionTimeCounter >= unit->reactionTime) {
		unit->reactionTimeCounter = 0;
		foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->attackRange,
												  game_spatial_filter_enemy_units, unit, foundUnitIds,
												  MAX_FOUND_UNITS);
		if (foundUnitsCount > 0) {
			GameUnit *target = game_unit_get_by_id(context, foundUnitIds[0]);
			if (target) game_unit_command_attack(unit, target, UNIT_STATE_DEFEND);
		}
	}
}

static void game_unit_ai_attack(GameContext *context, GameUnit *unit) {
	if (game_animation_unit_finished(unit)) {
		GameUnit *target = game_unit_get_by_id(context, unit->targetId);
		if (target && game_spatial_unit_in_range(unit, target, unit->attackRange)) {
            game_animation_unit_reset(unit);
		} else {
			unit->state = unit->nextState;
			unit->nextState = UNIT_STATE_IDLE;
		}
	}
}

#define MOVE_PRECISION 1024

static void game_unit_ai_move_anim(GameContext *context, GameUnit *unit) {
	if (++unit->moveTimeCounter > unit->moveTime) {
		unit->state = unit->nextState;
		unit->nextState = UNIT_STATE_IDLE;
		game_animation_unit_set(unit);
	}
}

static void game_unit_ai_move_attack(GameContext *context, GameUnit *unit) {
}

void game_unit_ai_invoke(GameContext *context, GameUnit *unit) {
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
	}
}

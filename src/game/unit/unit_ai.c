#include "unit_ai.h"

#define MAX_FOUND_UNITS 32
static UnitId foundUnitIds[MAX_FOUND_UNITS];
static uint16_t foundUnitsCount;

void game_unit_ai_idle(GameContext *context, GameUnit *unit) {
	if (++unit->reactionCurrentTime >= unit->reactionTime) {
		unit->reactionCurrentTime = 0;
		foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->attackRange,
												  game_spatial_filter_enemy_units, unit, foundUnitIds,
                                                  MAX_FOUND_UNITS);
		if (foundUnitsCount > 0) {
			GameUnit *target = game_unit_get_by_id(context, foundUnitIds[0]);
			if (target) {
				game_unit_command_attack(unit, target);
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

void game_unit_ai_invoke(GameContext *context, GameUnit *unit) {
	switch (unit->status) {
		case UNIT_STATUS_IDLE:
			game_unit_ai_idle(context, unit);
			break;
		case UNIT_STATUS_ATTACK:
			break;
		case UNIT_STATUS_DEFEND:
			break;
		case UNIT_STATUS_MOVE:
			break;
		case UNIT_STATUS_MOVE_ANIM:
			break;
		case UNIT_STATUS_MOVE_ATTACK:
			break;
	}
}

#include "unit_command.h"

static uint8_t game_unit_set_status_or_next(GameUnit* unit, UnitStatusEnum unitStatus) {
    if (unit->status == UNIT_STATUS_MOVE_ANIM) {
		unit->nextStatus = unitStatus;
        return FALSE;
	} else {
		unit->status = unitStatus;
        return TRUE;
	}
}

void game_unit_command_idle(GameUnit *unit) {
    unit->reactionCurrentTime = 0;
    game_unit_set_status_or_next(unit, UNIT_STATUS_IDLE);
}

void game_unit_command_attack(GameUnit *unit, GameUnit *target, UnitStatusEnum nextState) {
    // TODO: start animation counters
    unit->targetId = target->id;
    game_unit_set_status_or_next(unit, UNIT_STATUS_ATTACK);
}

void game_unit_command_defend(GameUnit *unit) {
    unit->reactionCurrentTime = 0;
    game_unit_set_status_or_next(unit, UNIT_STATUS_DEFEND);
}

void game_unit_command_move(GameUnit *unit, GameUnit *target, int16_t targetX, int16_t targetY) {
    unit->targetId = target->id;
    unit->targetX = targetX;
    unit->targetY = targetY;
    game_unit_set_status_or_next(unit, UNIT_STATUS_MOVE);
}

void game_unit_command_move_attack(GameUnit *unit, GameUnit *target, int16_t targetX, int16_t targetY) {
    unit->targetId = target->id;
    unit->targetX = targetX;
    unit->targetY = targetY;
    game_unit_set_status_or_next(unit, UNIT_STATUS_MOVE_ATTACK);
}

void game_unit_command_move_anim(GameUnit *unit, UnitStatusEnum nextState) {
    // TODO: start animation counters
    unit->status = UNIT_STATUS_MOVE_ANIM;
    unit->nextStatus = nextState;
}

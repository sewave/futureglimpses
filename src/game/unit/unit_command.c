#include "unit_command.h"

static uint8_t game_unit_set_state_or_next(GameUnit* unit, UnitStateEnum unitState) {
    if (unit->state == UNIT_STATE_MOVE_ANIM) {
		unit->nextState = unitState;
        return FALSE;
	} else {
		unit->state = unitState;
        return TRUE;
	}
}

void game_unit_command_idle(GameUnit *unit) {
    unit->reactionCurrentTime = 0;
    game_unit_set_state_or_next(unit, UNIT_STATE_IDLE);
}

void game_unit_command_attack(GameUnit *unit, GameUnit *target, UnitStateEnum nextState) {
    // TODO: start animation counters
    unit->stateCurrentCounter = 0;
    unit->targetId = target->id;
    game_unit_set_state_or_next(unit, UNIT_STATE_ATTACK);
}

void game_unit_command_defend(GameUnit *unit) {
    unit->reactionCurrentTime = 0;
    game_unit_set_state_or_next(unit, UNIT_STATE_DEFEND);
}

void game_unit_command_move(GameUnit *unit, GameUnit *target, int16_t targetX, int16_t targetY) {
    unit->targetId = target->id;
    unit->targetX = targetX;
    unit->targetY = targetY;
    game_unit_set_state_or_next(unit, UNIT_STATE_MOVE);
}

void game_unit_command_move_attack(GameUnit *unit, GameUnit *target, int16_t targetX, int16_t targetY) {
    unit->targetId = target->id;
    unit->targetX = targetX;
    unit->targetY = targetY;
    game_unit_set_state_or_next(unit, UNIT_STATE_MOVE_ATTACK);
}

void game_unit_command_move_anim(GameUnit *unit, UnitStateEnum nextState, uint16_t totalAnimationTime) {
    unit->stateCurrentCounter = 0;
    unit->stateFinalCounter = totalAnimationTime;
    unit->state = UNIT_STATE_MOVE_ANIM;
    unit->nextState = nextState;
}

#include "unit_command.h"

static uint8_t game_unit_set_state_or_next(GameUnit* unit, UnitStateEnum unitState) {
    if(unit->state == UNIT_STATE_DIE) return FALSE;
	if (unit->state == UNIT_STATE_MOVE_ANIM) {
		unit->nextState = unitState;
        return FALSE;
	} else {
		unit->state = unitState;
		game_animation_unit_set(unit);
		return TRUE;
	}
}

void game_unit_command_idle(GameUnit *unit) {
    unit->reactionTimeCounter = 0;
    game_unit_set_state_or_next(unit, UNIT_STATE_IDLE);
}

void game_unit_command_attack(GameUnit *unit, GameUnit *target, UnitStateEnum nextState) {
    unit->targetId = target->id;
    game_unit_face_target(unit, target);
    game_unit_set_state_or_next(unit, UNIT_STATE_ATTACK);
}

void game_unit_command_defend(GameUnit *unit) {
    unit->reactionTimeCounter = 0;
    game_unit_set_state_or_next(unit, UNIT_STATE_DEFEND);
}

void game_unit_command_move(GameUnit *unit, GameUnit *target, int16_t targetX, int16_t targetY) {
    if(target) unit->targetId = target->id; else unit->targetId = NO_TARGET_ID;
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

void game_unit_command_set_move_anim(GameUnit *unit, UnitStateEnum nextState) {
	unit->moveTimeCounter = 0;
    unit->state = UNIT_STATE_MOVE_ANIM;
    unit->nextState = nextState;
	game_animation_unit_set(unit);
}

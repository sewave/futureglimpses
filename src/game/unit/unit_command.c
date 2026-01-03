#include "unit_command.h"

static void game_unit_set_state_or_next(GameUnit* unit, UnitStateEnum unitState) {
    if(unit->state == UNIT_STATE_DIE) return;
    // If we are moving or attacking, we have to wait for it to finish
	if (unit->state == UNIT_STATE_MOVE_ANIM
        || (unit->state == UNIT_STATE_ATTACK && unitState == UNIT_STATE_MOVE_ATTACK)) {
		unit->nextState = unitState;
	} else {
		unit->state = unitState;
		game_animation_unit_set(unit);
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
    if(unit->isBuilding) return;
    unit->reactionTimeCounter = 0;
    game_unit_set_state_or_next(unit, UNIT_STATE_DEFEND);
}

void game_unit_command_move(GameUnit *unit, GameUnit *target, int16_t targetX, int16_t targetY) {
    if(unit->isBuilding) return;
    if(target) unit->targetId = target->id; else unit->targetId = NO_TARGET_ID;
    unit->targetX = targetX;
    unit->targetY = targetY;
    if(unit->type == UNIT_TYPE_WORKER && unit->typed.workerData.targetConstruction != NO_TARGET_ID) {
        if(target) {
            if(target->id != unit->typed.workerData.targetConstruction) {
                unit->typed.workerData.targetConstruction = NO_TARGET_ID;
            }
        } else {
            unit->typed.workerData.targetConstruction = NO_TARGET_ID;
        }
    }
    // TODO do the pathfinding?
    game_unit_set_state_or_next(unit, UNIT_STATE_MOVE);
}

void game_unit_command_move_attack(GameUnit *unit, GameUnit *target, int16_t targetX, int16_t targetY) {
    if(unit->isBuilding) return;
    if(target) {
        unit->targetId = target->id;
        unit->targetX = target->x;
        unit->targetY = target->y;
    }
    else {
        unit->targetX = targetX;
        unit->targetY = targetY;
    }
    game_unit_set_state_or_next(unit, UNIT_STATE_MOVE_ATTACK);
}

void game_unit_command_set_move_anim(GameUnit *unit, UnitStateEnum nextState) {
    if(unit->isBuilding) return;
	unit->moveTimeCounter = 0;
    unit->state = UNIT_STATE_MOVE_ANIM;
    unit->nextState = nextState;
	game_animation_unit_set(unit);
}

void game_unit_command_work(GameUnit* worker, GameUnit* target, int16_t targetX,  int16_t targetY) {
    if(worker->type != UNIT_TYPE_WORKER) return;
    // TODO work with target positions for harvesting
    if(target) {
        worker->typed.workerData.targetConstruction = target->id;
    }
    game_unit_set_state_or_next(worker, UNIT_STATE_WORK);
}

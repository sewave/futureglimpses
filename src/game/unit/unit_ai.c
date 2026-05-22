#include "game/unit/unit_ai.h"

#define MAX_FOUND_UNITS 8
static GameUnit *foundUnits[MAX_FOUND_UNITS];

static void game_unit_ai_idle(GameContext *context, GameUnit *unit) {
	if (unit->reactionTimeCounter >= unit->reactionTime) {
		unit->reactionTimeCounter = 0;
		uint16_t foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->maxAttackRange,
														   game_spatial_filter_enemy_units_in_attack_range, unit, foundUnits,
														   1);
		if (foundUnitsCount > 0) {
			game_unit_command_attack(unit, foundUnits[0], UNIT_STATE_IDLE);
		} else {
			foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->sightRange,
													  game_spatial_filter_enemy_units, unit, foundUnits,
													  MAX_FOUND_UNITS);
			if (foundUnitsCount > 0) {
				// Target nearest unit
				GameUnit *target = NULL;
				int distance = 9999999;
				GameUnit **newTargetList = foundUnits;
				for (int i = 0; i < foundUnitsCount; i++, newTargetList++) {
					GameUnit *newTarget = *newTargetList;
					int newDistance = distance_sq(unit->x, unit->y, newTarget->x, newTarget->y);
					if (newDistance < distance) {
						target = newTarget;
						distance = newDistance;
					}
				}
				if (target) game_unit_command_move_attack(unit, NULL, target->x, target->y);
			} else {
				// We found nothing, so we change direction to make it "look" around
				unit->direction = (unit->direction + 1) % DIRECTIONS_COUNT;
				game_animation_unit_set(unit);
			}
		}
	}
}

static void game_unit_ai_idle_worker(GameContext *context, GameUnit *worker) {
	if (worker->reactionTimeCounter >= worker->reactionTime) {
		worker->reactionTimeCounter = 0;
		WorkerData *workerData = &worker->typed.workerData;
		// If we have a targetWorkingBuilding, we must approach it
		if (workerData->targetConstruction != NO_TARGET_ID) {
			GameUnit *targetBuilding = game_unit_get_by_id(context, workerData->targetConstruction);
			if (targetBuilding) {
				// Check sorroundings, if we are touching building, start working
				if (game_spatial_unit_around_position(context, targetBuilding->id, worker->x, worker->y)) {
					game_unit_command_work(worker, targetBuilding, NO_TARGET_POSITION, NO_TARGET_POSITION);
				} else {
					game_unit_command_move(worker, targetBuilding, NO_TARGET_POSITION, NO_TARGET_POSITION);
				}
			} else {
				// Building no longer exists, cancel
				workerData->targetConstruction = NO_TARGET_ID;
			}
		} else {
			// If we area near our workplace, start to work
			if (workerData->workplace.x != NO_TARGET_POSITION && workerData->workplace.y != NO_TARGET_POSITION) {
				// If workplace is no longer a resource, cancel workplace
				BoardTile *workplaceTile = &context->board[workerData->workplace.x][workerData->workplace.y];
				if (workplaceTile->type != TILE_TYPE_GOLD && workplaceTile->type != TILE_TYPE_WOOD) {
					workerData->workplace.x = NO_TARGET_POSITION;
					workerData->workplace.y = NO_TARGET_POSITION;
					resource_search_for_work(context, worker);
				} else {
					if (game_spatial_unit_around_position(context, worker->id, workerData->workplace.x, workerData->workplace.y)) {
						game_unit_command_work(worker, NULL, workerData->workplace.x, workerData->workplace.y);
					} else {
						game_unit_command_move(worker, NULL, workerData->workplace.x, workerData->workplace.y);
					}
					return;
				}
			}
			if(worker->state == UNIT_STATE_IDLE) {
				// Idle, so we change direction to make it "look" around
				worker->direction = (worker->direction + 1) % DIRECTIONS_COUNT;
			}
			game_animation_unit_set(worker);
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

	if (unit->type == UNIT_TYPE_WORKER) {
		uint8_t mustWork = FALSE;
		WorkerData *workerData = &unit->typed.workerData;
		if (workerData->targetConstruction != NO_TARGET_ID) {
			GameUnit *workUnit = game_unit_get_by_id(context, workerData->targetConstruction);
			if (workUnit && game_spatial_unit_around_position(context, workUnit->id, unit->x, unit->y)) {
				game_unit_command_work(unit, workUnit, NO_TARGET_POSITION, NO_TARGET_POSITION);
				mustWork = TRUE;
			}
		}
		if (workerData->carriedResourceQty > 0 && unit->targetId != NO_TARGET_ID) {
			GameUnit *destination = game_unit_get_by_id(context, unit->targetId);
			if (destination && destination->type == UNIT_TYPE_CITY_HALL && game_spatial_unit_around_position(context, destination->id, unit->x, unit->y)) {
				resource_add_amount(context, unit->controller, workerData->carriedResourceType, workerData->carriedResourceQty);
				workerData->carriedResourceQty = 0;
				if(workerData->workplace.x != NO_TARGET_POSITION && workerData->workplace.y != NO_TARGET_POSITION && !mustWork) {
					game_unit_command_move(unit, NULL, workerData->workplace.x, workerData->workplace.y);
					return;
				}
			}
		}
		if(mustWork) return;
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
	if (unit->reactionTimeCounter >= unit->reactionTime) {
		unit->reactionTimeCounter = 0;
		uint16_t foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->maxAttackRange,
														   game_spatial_filter_enemy_units, unit, foundUnits,
														   1);
		if (foundUnitsCount > 0) game_unit_command_attack(unit, foundUnits[0], UNIT_STATE_DEFEND);
	}
}

static void game_unit_ai_attack(GameContext *context, GameUnit *unit) {
	GameUnit *target = game_unit_get_by_id(context, unit->targetId);
	if (game_animation_finished(&unit->animationStatus) || !target || target->state == UNIT_STATE_DIE) {
		if (target && target->state != UNIT_STATE_DIE && game_spatial_unit_target_in_attack_range(unit, target)) {
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
	if (unit->moveTimeCounter > unit->moveTimeAnim) {
		unit->prevX = unit->x;
		unit->prevY = unit->y;
		unit->state = unit->nextState;
		unit->nextState = UNIT_STATE_IDLE;
		if(unit->controller == UNIT_CONTROLLER_PLAYER) game_unit_explore(context, unit);
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
		if(unit->targetId == NO_TARGET_ID) {
			game_unit_command_idle(unit);
			return;
		}
		targetUnit = game_unit_get_by_id(context, unit->targetId);
		if (!targetUnit) {
			game_unit_command_idle(unit);
			unit->targetId = NO_TARGET_ID;
			return;
		}
		targetX = targetUnit->x + (targetUnit->tileSize - 1) / 2;
		targetY = targetUnit->y + (targetUnit->tileSize - 1) / 2;
	}

	if (unit->x == targetX && unit->y == targetY) {
		game_unit_command_idle(unit);
		return;
	}

	if (targetUnit && game_spatial_unit_target_in_attack_range(unit, targetUnit)) {
		game_unit_command_attack(unit, targetUnit, UNIT_STATE_IDLE);
		return;
	}

	// If we are not attacking a concrete unit, fight against any enemy unit in the way
	if(!targetUnit) {
		uint16_t foundUnitsCount = game_spatial_query_grid(context, unit->x, unit->y, unit->maxAttackRange,
														game_spatial_filter_enemy_units_in_attack_range, unit, foundUnits, 1);
		if (foundUnitsCount > 0) {
			game_unit_command_attack(unit, foundUnits[0], UNIT_STATE_MOVE_ATTACK);
			return;
		}
	}

	if (game_unit_path_find(context, unit, targetX, targetY)) {
		game_unit_command_set_move_anim(unit, UNIT_STATE_MOVE_ATTACK);
	}
}

static void game_unit_ai_die(GameContext *context, GameUnit *unit) {
	if (game_animation_finished(&unit->animationStatus)) game_unit_destroy(context, unit->id);
}

static void game_unit_ai_work_worker(GameContext *context, GameUnit *worker) {
	if (game_animation_finished(&worker->animationStatus)) {
		UnitId targetBuildingId = worker->typed.workerData.targetConstruction;
		if (targetBuildingId != NO_TARGET_ID) {
			GameUnit *targetBuilding = game_unit_get_by_id(context, targetBuildingId);
			if (targetBuilding &&
				(targetBuilding->state == BUILDING_STATE_CONSTRUCT || targetBuilding->health < targetBuilding->maxHealth)) {
				game_animation_reset(&worker->animationStatus);
			} else {
				GameUnit *constructionSite = game_unit_get_nearest_construction_building(context, worker);
				if(constructionSite) {
					worker->typed.workerData.targetConstruction = constructionSite->id;
					game_unit_command_move(worker, constructionSite, NO_TARGET_POSITION, NO_TARGET_POSITION);
				}
				else {
					worker->typed.workerData.targetConstruction = NO_TARGET_ID;
					game_unit_command_idle(worker);
				}
			}
		} else {
			WorkerData *workerData = &worker->typed.workerData;
			if (workerData->workplace.x != NO_TARGET_POSITION &&
				workerData->workplace.y != NO_TARGET_POSITION) {
				BoardTile *tile = &context->board[workerData->workplace.x][workerData->workplace.y];
				// If there are still resources, keep working, otherwise stop
				if((tile->type == TILE_TYPE_WOOD || tile->type == TILE_TYPE_GOLD) && tile->data > 0) {
					game_animation_reset(&worker->animationStatus);
				}
				else {
					workerData->workplace.x = NO_TARGET_POSITION;
					workerData->workplace.y = NO_TARGET_POSITION;
					game_unit_command_idle(worker);
				}
			}
			else {
				game_unit_command_idle(worker);
			}
		}
	}
}

void game_unit_ai_invoke(GameContext *context, GameUnit *unit) {
	if (unit->isBuilding) {
		if (unit->state == UNIT_STATE_DIE) game_unit_ai_die(context, unit);
		if (unit->state == BUILDING_STATE_COMPLETED) building_update(context, unit);
	} else {
		switch (unit->state) {
			case UNIT_STATE_IDLE:
				if (unit->type == UNIT_TYPE_WORKER) {
					game_unit_ai_idle_worker(context, unit);
				} else {
					game_unit_ai_idle(context, unit);
				}
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
				if (unit->type == UNIT_TYPE_WORKER) game_unit_ai_work_worker(context, unit);
				break;
			case UNIT_STATE_DIE:
				game_unit_ai_die(context, unit);
				break;
			case UNIT_STATES_COUNT:
				// Nothing, to disable warning
				break;
		}
	}
}

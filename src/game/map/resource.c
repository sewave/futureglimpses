#include "game/map/resource.h"
#include <stdio.h>

#define UI_RESOURCE_UPDATE_RATE 20 // Quantity to update of ui resource per call
#define BASIC_RESOURCES 2
#define PRINTED_RESOURCES 3
#define RESOURCES_Z UI_Z_ORDER + 600
#define BASE_TEN_NUMER 10
#define FOOD_USE_FORMAT "%u/%u"
#define FOOD_USE_SURPASSED_FORMAT "^005%u^001/%u"
#define WORKER_RESOURCE_WOOD_GATHER 3
#define WORKER_RESOURCE_GOLD_GATHER 10
#define WORKER_RESOURCE_GATHER_MAX 100

static int RESOURCE_TEXT_LOCATIONS[PRINTED_RESOURCES][2] = {
		{89, 1},
		{136, 1},
		{186, 1}};
static int RESOURCE_ICONS_LOCATIONS[PRINTED_RESOURCES][2] = {
		{79, 2},
		{126, 2},
		{176, 2}};

static char resourceBuffers[RESOURCE_TYPES_COUNT][16];

void resource_reset(GameContext *context) {
	for (int i = 0; i < UNIT_CONTROLLERS_COUNT; i++) {
		for (int j = 0; j < RESOURCE_TYPES_COUNT; j++) {
			context->resources[i].quantity[j] = 0;
			context->resources[i].uiQuantity[j] = 0;
		}
	}
}

void resource_set_amount(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount) {
	if (controller >= UNIT_CONTROLLERS_COUNT || type >= RESOURCE_TYPES_COUNT) return;
	context->resources[controller].quantity[type] = amount;
	context->resources[controller].uiQuantity[type] = amount;
}

uint8_t resource_has_enough(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount) {
	if (controller >= UNIT_CONTROLLERS_COUNT || type >= RESOURCE_TYPES_COUNT) return FALSE;
	return context->resources[controller].quantity[type] >= amount;
}

void resource_add_amount(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount) {
	if (controller >= UNIT_CONTROLLERS_COUNT || type >= RESOURCE_TYPES_COUNT) return;
	context->resources[controller].quantity[type] += amount;
	context->stats[controller].resourcesGathered[type] += amount;
}

void resource_deduct_amount(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount) {
	if (controller >= UNIT_CONTROLLERS_COUNT || type >= RESOURCE_TYPES_COUNT) return;
	if (context->resources[controller].quantity[type] >= amount) {
		context->resources[controller].quantity[type] -= amount;
	} else {
		context->resources[controller].quantity[type] = 0;
	}
	context->stats[controller].resourcesSpent[type] += amount;
}

static void resource_update_available_food(GameContext *context, ControllerEnum controller) {
	if(controller >= UNIT_CONTROLLERS_COUNT) return;
	uint32_t maxFood = context->resources[controller].quantity[RESOURCE_TYPE_MAX_FOOD];
	uint32_t usedFood = context->resources[controller].quantity[RESOURCE_TYPE_USED_FOOD];
	uint32_t availableFood;
	if(usedFood < maxFood) {
		availableFood = maxFood - usedFood;
	}
	else {
		availableFood = 0;
	}
	resource_set_amount(context, controller, RESOURCE_TYPE_AVAILABLE_FOOD, availableFood);
}

void resource_add_food_usage(GameContext *context, ControllerEnum controller, uint8_t foodUsage) {
	resource_add_amount(context, controller, RESOURCE_TYPE_USED_FOOD, foodUsage);
	resource_update_available_food(context, controller);
}

void resource_add_food_provided(GameContext *context, ControllerEnum controller, uint8_t foodProvided) {
	resource_add_amount(context, controller, RESOURCE_TYPE_MAX_FOOD, foodProvided);
	resource_update_available_food(context, controller);
}

void resource_deduct_food(GameContext *context, ControllerEnum controller, uint8_t foodUsage, uint8_t foodProvided) {
	resource_deduct_amount(context, controller, RESOURCE_TYPE_MAX_FOOD, foodProvided);
	resource_deduct_amount(context, controller, RESOURCE_TYPE_USED_FOOD, foodUsage);
	resource_update_available_food(context, controller);
}

void resource_update_ui_quantities(GameContext *context) {
	for (int i = 0; i < UNIT_CONTROLLERS_COUNT; i++) {
		for (int j = 0; j < RESOURCE_TYPES_COUNT; j++) {
			uint32_t actual = context->resources[i].quantity[j];
			uint32_t *uiQuantity = &context->resources[i].uiQuantity[j];
			if (*uiQuantity < actual) {
				uint32_t diff = actual - *uiQuantity;
				if (diff > UI_RESOURCE_UPDATE_RATE) {
					*uiQuantity += UI_RESOURCE_UPDATE_RATE;
				} else {
					*uiQuantity = actual;
				}
			} else {
				if (*uiQuantity > actual) {
					uint32_t diff = *uiQuantity - actual;
					if (diff > UI_RESOURCE_UPDATE_RATE) {
						*uiQuantity -= UI_RESOURCE_UPDATE_RATE;
					} else {
						*uiQuantity = actual;
					}
				}
			}
		}
	}
}

void resource_render_queue_submit_ui(GameContext *context, RenderQueue* renderQueue) {
	// Resource icons
	for (int i = 0; i < PRINTED_RESOURCES; i++) {
		render_queue_submit_sprite(
				renderQueue, RESOURCES_Z, game_gfx_get_icon(i), RESOURCE_ICONS_LOCATIONS[i][0],
				RESOURCE_ICONS_LOCATIONS[i][1], RND_FLAG_NORMAL);
	}

	// Render resources
	for (int i = 0; i < BASIC_RESOURCES; i++) {
		itoa(context->resources[UNIT_CONTROLLER_PLAYER].uiQuantity[i], resourceBuffers[i], BASE_TEN_NUMER);
		render_queue_submit_text_shadow(renderQueue, RESOURCES_Z,
				context->gameFont, resourceBuffers[i],
				RESOURCE_TEXT_LOCATIONS[i][0], RESOURCE_TEXT_LOCATIONS[i][1],
				PAL_COLOR_WHITE, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
	}

	// Render food usage
	uint32_t usedFood = context->resources[UNIT_CONTROLLER_PLAYER].uiQuantity[RESOURCE_TYPE_USED_FOOD];
	uint32_t maxFood = context->resources[UNIT_CONTROLLER_PLAYER].uiQuantity[RESOURCE_TYPE_MAX_FOOD];
	char* foodFormat;
	if(usedFood > maxFood) {
		foodFormat = FOOD_USE_SURPASSED_FORMAT;
	} else {
		foodFormat = FOOD_USE_FORMAT;
	}
	snprintf(resourceBuffers[RESOURCE_TYPE_AVAILABLE_FOOD], sizeof(resourceBuffers[RESOURCE_TYPE_AVAILABLE_FOOD]),
			 foodFormat, usedFood, maxFood);
	render_queue_submit_text_multicolor_shadow(renderQueue, RESOURCES_Z, context->gameFont,
			resourceBuffers[RESOURCE_TYPE_AVAILABLE_FOOD],
			RESOURCE_TEXT_LOCATIONS[RESOURCE_TYPE_AVAILABLE_FOOD][0],
			RESOURCE_TEXT_LOCATIONS[RESOURCE_TYPE_AVAILABLE_FOOD][1],
			PAL_COLOR_WHITE, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
}

void resource_unit_harvest(GameContext *context, GameUnit *worker) {
	if (worker->type != UNIT_TYPE_WORKER) return;
	WorkerData *workerData = &worker->typed.workerData;
	if (workerData->workplace.x != NO_TARGET_POSITION && workerData->workplace.y != NO_TARGET_POSITION) {
		BoardTile *tile = &context->board[workerData->workplace.x][workerData->workplace.y];
		switch (tile->type) {
			case TILE_TYPE_WOOD:
				if (workerData->carriedResourceType == RESOURCE_TYPE_WOOD || workerData->carriedResourceQty == 0) {
					workerData->carriedResourceType = RESOURCE_TYPE_WOOD;
					workerData->carriedResourceQty = clamp(workerData->carriedResourceQty + WORKER_RESOURCE_WOOD_GATHER, 0,
														   WORKER_RESOURCE_GATHER_MAX);
				}
				if (tile->data >= WORKER_RESOURCE_WOOD_GATHER) {
					tile->data -= WORKER_RESOURCE_WOOD_GATHER;
				} else {
					tile->data = 0;
				}
				break;
			case TILE_TYPE_GOLD:
				if (workerData->carriedResourceType == RESOURCE_TYPE_GOLD || workerData->carriedResourceQty == 0) {
					workerData->carriedResourceType = RESOURCE_TYPE_GOLD;
					workerData->carriedResourceQty = clamp(workerData->carriedResourceQty + WORKER_RESOURCE_GOLD_GATHER, 0,
														   WORKER_RESOURCE_GATHER_MAX);
				}
				if (tile->data >= WORKER_RESOURCE_GOLD_GATHER) {
					tile->data -= WORKER_RESOURCE_GOLD_GATHER;
				} else {
					tile->data = 0;
				}
				break;
			default:
				return;
				break;
		}
		if (tile->data == 0) {
			tile->type = TILE_TYPE_WALKABLE;
			tile->tile = tile->altTile;
			context->walkabilityGrid[workerData->workplace.x][workerData->workplace.y] = WALKABILITY_FREE;
			if(context->boardExploration[workerData->workplace.x][workerData->workplace.y] == BOARD_EXPLORED) {
				blit(game_gfx_get_tileset(), context->renderedBoard,
					(tile->tile % TILE_SIZE) * TILE_SIZE, (tile->tile / TILE_SIZE) * TILE_SIZE,
					workerData->workplace.x * TILE_SIZE, workerData->workplace.y * TILE_SIZE,
					TILE_SIZE, TILE_SIZE);
				putpixel(context->renderedMinimap, workerData->workplace.x, workerData->workplace.y, context->minimapColors[tile->tile]);
			}
		}

		if (workerData->carriedResourceQty >= WORKER_RESOURCE_GATHER_MAX) {
			// We set our current position as target to search sorroundings
			workerData->workplace.x = worker->x;
			workerData->workplace.y = worker->y;

			// Search the nearest city hall on active units to drop the resources
			GameUnit *closeCityHall = game_unit_get_nearest_unit_type(context, worker, UNIT_TYPE_CITY_HALL, worker->controller);
			if (closeCityHall) game_unit_command_move(worker, closeCityHall, closeCityHall->x, closeCityHall->y);
		}
		else {
			if(tile->data == 0) {
				workerData->workplace.x = NO_TARGET_POSITION;
				workerData->workplace.y = NO_TARGET_POSITION;
				resource_search_for_work(context, worker);
			}
		}
	}
}

static Position resource_find_first_around_unit(GameContext *context, GameUnit *unit, TileTypeEnum resourceType, uint8_t maxDistance) {
	// Search in range for closest resource using distance_sq to avoid sqrt
	int closestDistanceSq = maxDistance * maxDistance + 1;
	Position closestResourcePos = {.x = NO_TARGET_POSITION, .y = NO_TARGET_POSITION};
	for (int dx = -maxDistance; dx <= maxDistance; dx++) {
		for (int dy = -maxDistance; dy <= maxDistance; dy++) {
			int checkX = unit->x + dx;
			int checkY = unit->y + dy;
			if (checkX < 0 || checkX >= BOARD_WIDTH || checkY < 0 || checkY >= BOARD_HEIGHT) continue;
			BoardTile *tile = &context->board[checkX][checkY];
			if (tile->type == resourceType) {
				int distanceSq = dx * dx + dy * dy;
				if (distanceSq < closestDistanceSq) {
					closestDistanceSq = distanceSq;
					closestResourcePos.x = checkX;
					closestResourcePos.y = checkY;
				}
			}
		}
	}
	return closestResourcePos;
}

void resource_search_for_work(GameContext *context, GameUnit *worker) {
	if (worker->type != UNIT_TYPE_WORKER) return;
	WorkerData *workerData = &worker->typed.workerData;
	TileTypeEnum searchType = workerData->carriedResourceType != RESOURCE_TYPE_NONE ? (workerData->carriedResourceType == RESOURCE_TYPE_WOOD ? TILE_TYPE_WOOD : TILE_TYPE_GOLD) : TILE_TYPE_WOOD;
	Position nearResource = resource_find_first_around_unit(context, worker, searchType, worker->sightRange);
	if (nearResource.x == NO_TARGET_POSITION && nearResource.y == NO_TARGET_POSITION) {
		searchType = searchType == TILE_TYPE_WOOD ? TILE_TYPE_GOLD : TILE_TYPE_WOOD;
		nearResource = resource_find_first_around_unit(context, worker, searchType, worker->sightRange);
	}

	if (nearResource.x != NO_TARGET_POSITION && nearResource.y != NO_TARGET_POSITION) {
		workerData->workplace = nearResource;
		game_unit_command_move(worker, NULL, nearResource.x, nearResource.y);
	}
}

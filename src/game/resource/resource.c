#include "resource.h"

#define UI_RESOURCE_UPDATE_RATE 20 // Quantity to update of ui resource per call
#define BASIC_RESOURCES 2
#define PRINTED_RESOURCES 3
#define RESOURCES_Z UI_Z_ORDER + 600
#define BASE_TEN_NUMER 10
#define FOOD_USE_FORMAT "%u/%u"

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
	snprintf(resourceBuffers[RESOURCE_TYPE_AVAILABLE_FOOD], sizeof(resourceBuffers[RESOURCE_TYPE_AVAILABLE_FOOD]),
			 FOOD_USE_FORMAT, context->resources[UNIT_CONTROLLER_PLAYER].uiQuantity[RESOURCE_TYPE_USED_FOOD],
			 context->resources[UNIT_CONTROLLER_PLAYER].uiQuantity[RESOURCE_TYPE_MAX_FOOD]);
	render_queue_submit_text_shadow(renderQueue, RESOURCES_Z, context->gameFont,
			resourceBuffers[RESOURCE_TYPE_AVAILABLE_FOOD],
			RESOURCE_TEXT_LOCATIONS[RESOURCE_TYPE_AVAILABLE_FOOD][0],
			RESOURCE_TEXT_LOCATIONS[RESOURCE_TYPE_AVAILABLE_FOOD][1],
			PAL_COLOR_WHITE, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
}
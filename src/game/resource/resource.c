#include "resource.h"

#define UI_RESOURCE_UPDATE_RATE 4// QUantity to update of ui resource per call

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

void resource_add_food(GameContext *context, ControllerEnum controller, uint8_t foodUsage, uint8_t foodProvided) {
	resource_add_amount(context, controller, RESOURCE_TYPE_MAX_FOOD, foodProvided);
	resource_add_amount(context, controller, RESOURCE_TYPE_USED_FOOD, foodUsage);
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

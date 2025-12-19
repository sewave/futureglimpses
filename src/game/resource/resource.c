#include "resource.h"

#define UI_RESOURCE_UPDATE_RATE 4 // QUantity to update of ui resource per call

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


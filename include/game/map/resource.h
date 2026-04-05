#ifndef RESOURCE_H
#define RESOURCE_H
#include "game/game_lib.h"

void resource_reset(GameContext *context);
void resource_set_amount(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount);
uint8_t resource_has_enough(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount);
void resource_add_amount(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount);
void resource_deduct_amount(GameContext *context, ControllerEnum controller, ResourceTypeEnum type, uint32_t amount);
void resource_update_ui_quantities(GameContext *context);
void resource_add_food_usage(GameContext *context, ControllerEnum controller, uint8_t foodUsage);
void resource_add_food_provided(GameContext *context, ControllerEnum controller, uint8_t foodProvided);
void resource_deduct_food(GameContext *context, ControllerEnum controller, uint8_t foodUsage, uint8_t foodProvided);
void resource_render_queue_submit_ui(GameContext *context, RenderQueue* renderQueue);
void resource_unit_harvest(GameContext *context, GameUnit *worker);
void resource_search_for_work(GameContext *context, GameUnit *worker);
Position resource_find_first_around_unit(GameContext *context, GameUnit *unit, TileTypeEnum resourceType, uint8_t maxDistance);
uint8_t resource_has_enough_for_unit(GameContext *context, ControllerEnum controller, UnitTypeEnum unitType);
#endif /* RESOURCE_H */

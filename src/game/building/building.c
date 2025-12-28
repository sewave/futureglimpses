#include "building.h"

typedef struct {
	uint16_t resources[RESOURCE_TYPES_COUNT];
	uint16_t time;
} UnitResourcesData;

static UnitResourcesData UNIT_RESOURCES[UNIT_TYPE_NUMBER] = {
		// WORKER
		{.resources = {1, 0}, .time = SEC_TO_FRAMES(1)},
		// SOLDIER
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// ARCHER
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// KNIGHT
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// MAGE
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// UNIT_TYPE_CITY_HALL
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// UNIT_TYPE_FARM
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// UNIT_TYPE_BARRACKS
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// UNIT_TYPE_BLACKSMITH
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// UNIT_TYPE_STABLES
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
		// UNIT_TYPE_TOWER
		{.resources = {0, 0}, .time = SEC_TO_FRAMES(10)},
};

void building_add_to_train_queue(GameContext *context, GameUnit *building, UnitTypeEnum unitType) {
	// Check funds
	for (int i = 0; i < RESOURCE_TYPES_COUNT; i++) {
		if (!resource_has_enough(context, building->controller, i, UNIT_RESOURCES[unitType].resources[i])) {
			// TODO Queue not enought XXX message, free for now
			return;
		}
	}

    // Deduct resources
    for (int i = 0; i < RESOURCE_TYPES_COUNT; i++) {
		resource_deduct_amount(context, building->controller, i, UNIT_RESOURCES[unitType].resources[i]);
	}

	// If building is free, start training
	BuildingData *buildingData = &building->typed.buildingData;

	if (buildingData->isTraining) {
		// TODO add to internal queue, if no free space, add message to queue messages
	} else {
		buildingData->isTraining = TRUE;
		buildingData->trainUnit = unitType;
        buildingData->currentTrainTicks = 0;
        buildingData->targetTrainTicks = UNIT_RESOURCES[unitType].time;
	}
}

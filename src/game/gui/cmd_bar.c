#include "game/gui/cmd_bar.h"
#include <stdio.h>
#include <allegro/keyboard.h>

#define CMD_BAR_BUTTON_INITIAL_X 2
#define CMD_BAR_BUTTON_INITIAL_Y 129

#define CMD_BAR_BUTTON_SEPARATION_WIDTH 35
#define CMD_BAR_BUTTON_SEPARATION_HEIGHT 24
#define CMD_BAR_BUTTON_LINE_WIDTH CMD_BAR_BUTTON_WIDTH - 2
#define CMD_BAR_BUTTON_LINE_HEIGTH CMD_BAR_BUTTON_HEIGHT - 2

#define CMD_BUTTON_Z UI_Z_ORDER + 505
#define CMD_BUTTON_HOVER_Z UI_Z_ORDER + 505
#define CMD_BUTTON_DOWN_RECT_Z UI_Z_ORDER + 506
#define CMD_BUTTON_DOWN_LINE_Z UI_Z_ORDER + 507

#define UNIT_SHEET_COL_ONE_X 3
#define UNIT_SHEET_COL_TWO_X 42
#define UNIT_SHEET_ROW_ONE_Y 80
#define UNIT_SHEET_ROW_TWO_Y 90
#define UNIT_SHEET_ROW_THREE_Y 104
#define UNIT_SHEET_ROW_THREE_Y_TEXT_OFFSET 1
#define UNIT_SHEET_ROW_FOUR_Y 117
#define UNIT_SHEET_HP_BAR_X UNIT_SHEET_COL_ONE_X + 3
#define UNIT_SHEET_TRAIN_BAR_X UNIT_SHEET_COL_ONE_X + 3
#define UNIT_SHEET_BACK_X 1
#define UNIT_SHEET_BACK_Y 79
#define UNIT_SHEET_BACK_WIDTH 68
#define UNIT_SHEET_BACK_HEIGHT 48

#define UNIT_SHEET_BAR_LENGTH 58
#define UNIT_SHEET_BAR_HEIGHT 11
#define UNIT_SHEET_BAR_TEXT_Y_OFF 1
#define UNIT_SHEET_Z_ORDER_BACKGROUND UI_Z_ORDER + 505
#define UNIT_SHEET_Z_ORDER_BAR UI_Z_ORDER + 509
#define UNIT_SHEET_Z_ORDER_SHEET_TEXT UI_Z_ORDER + 510
#define UNIT_SHEET_Z_ORDER_BAR_RECT UI_Z_ORDER + 511

#define NO_UNIT_RESOURCES 2
#define RESOURCES_Z UI_Z_ORDER + 600
#define RESOURCE_LOCATIONS_TEXT_X_OFF 10
#define RESOURCE_LOCATIONS_TEXT_Y_OFF -1
#define NO_UNIT_RESOURCE_X_OFF 28

#define RANGE_SEPARATOR "-"
#define HEALTH_SEPARATOR "/"
#define UNITS_PERCENT " \%"

static Position RESOURCE_LOCATIONS[UNIT_USED_RESOURCES] = {
		{220, 190},
		{258, 190},
		{295, 190}};

static uint8_t buttonIndex;
static uint8_t unitCount[UNIT_TYPE_NUMBER] = {0};
static uint8_t trainings[TRAINING_TYPE_NUMBER] = {0};
static char unitHpText[16];
static char unitDamageText[16];
static char unitAtRangeText[16];
static char unitArmorText[16];
static char unitsText[16];
static char unitsResQty[16];
static char resourceBuffers[RESOURCE_TYPES_COUNT][8];

static void game_cmd_bar_clear_build_placing(BuildPlacing *buildPlacing) {
	buildPlacing->state = CMD_BAR_BUILD_STATE_NONE;
	buildPlacing->showBuilding = FALSE;
}

static void handle_train_unit(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	if (context->selectedUnitCount == 1) {
		GameUnit *building = game_unit_get_by_id(context, context->selectedUnits[0]);
		if (building) building_add_to_train_queue(context, building, (TrainingTypeEnum) fixedDat);
	}
}

static void return_resources_for_cancelled_unit(GameContext *context, ControllerEnum controller, TrainingTypeEnum trainingType) {
	TrainingResourcesData *unitResources = game_unit_get_training_resources(trainingType);
	for (int i = 0; i < UNIT_CREATE_REDUCE_RESOURCES; i++) {
		resource_add_amount(context, controller, i, unitResources->used[i]);
	}
}

static void handle_cancel_all_unit_training(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	if (context->selectedUnitCount == 1) {
		GameUnit *building = game_unit_get_by_id(context, context->selectedUnits[0]);
		if (building) {
			// Cancel all the queue and return resources for all the units in the queue,
			// including the one in training
			if(building->typed.buildingData.isTraining) {
				return_resources_for_cancelled_unit(context, building->controller, building->typed.buildingData.trainingType);
				building->typed.buildingData.isTraining = FALSE;
			}

			for (int i = 0; i < building->typed.buildingData.queueNextIndex; i++) {
				return_resources_for_cancelled_unit(context, building->controller, building->typed.buildingData.queue[i]);
			}
			building->typed.buildingData.queueNextIndex = 0;
		}
	}
}

static void handle_cancel_button(void *ctxVoid, uint8_t fixedDat) {
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
}

static void handle_building_select_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	context->buildPlacing.state = CMD_BAR_BUILD_STATE_SELECT;
}

static void handle_building_select_advanced_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	context->buildPlacing.state = CMD_BAR_BUILD_STATE_SELECT_ADVANCED;
}

static void handle_build_place_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	UnitTypeEnum buildingType = (UnitTypeEnum) fixedDat;

    TrainingResourcesData* unitResources = game_unit_get_training_resources(buildingType);
	uint8_t canPlace = TRUE;
	for (int i = 0; i < UNIT_USED_RESOURCES; i++) {
		if (!resource_has_enough(context, UNIT_CONTROLLER_PLAYER, i, unitResources->used[i])) {
				message_add_to_queue_shadow(text_get_by_id(GAME_TEXT_ID_NOT_ENOUGH_GOLD + i),
								 NOT_ENOUGH_RESOURCE_TIME, PAL_COLOR_YELLOW, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
				game_snd_play_sound(GAME_SOUND_NOT_VALID);
			canPlace = FALSE;
			break;
		}
	}

	if (canPlace) {
		context->buildPlacing.state = CMD_BAR_BUILD_STATE_PLACE;
		context->buildPlacing.building = buildingType;
	}
}

static void handle_build_select_cancel_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	game_cmd_bar_clear_build_placing(&context->buildPlacing);
}

static void handle_build_place_cancel_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	context->buildPlacing.state = CMD_BAR_BUILD_STATE_NONE;
}

static void handle_build_cancel_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[0]);
	if (unit) {
		// Return used resources
		TrainingResourcesData *unitResources = game_unit_get_training_resources(unit->type);
		for (int i = 0; i < UNIT_CREATE_REDUCE_RESOURCES; i++) {
			resource_add_amount(context, unit->controller, i, unitResources->used[i]);
		}
		// Destroy the building
		game_unit_destroy(context, unit->id);
	}
}

static void handle_action_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	switch (fixedDat) {
		case UNIT_STATE_MOVE:
			game_mouse_set_cursor_state(MOUSE_CURSOR_TARGET);
			break;
		case UNIT_STATE_ATTACK:
			game_mouse_set_cursor_state(MOUSE_CURSOR_ATTACK);
			break;
		case UNIT_STATE_IDLE:
			for (int i = 0; i < context->selectedUnitCount; i++) {
				GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
				if (unit) game_unit_command_idle(unit);
			}
			break;
		case UNIT_STATE_DEFEND:
			for (int i = 0; i < context->selectedUnitCount; i++) {
				GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
				if (unit) game_unit_command_defend(unit);
			}
			break;
	}
}

static const CommandBarButton MOVE_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_M,
		.hotkey = "M",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_MOVE,
		.fixedParam = UNIT_STATE_MOVE,
		.icon = CMD_BAR_BUTTON_ICON_MOVE,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton STOP_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_S,
		.hotkey = "S",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_STOP,
		.fixedParam = UNIT_STATE_IDLE,
		.icon = CMD_BAR_BUTTON_ICON_STOP,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton ATTACK_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_A,
		.hotkey = "A",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_ATTACK,
		.fixedParam = UNIT_STATE_ATTACK,
		.icon = CMD_BAR_BUTTON_ICON_ATTACK,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton DEFEND_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_D,
		.hotkey = "D",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_DEFEND,
		.fixedParam = UNIT_STATE_DEFEND,
		.icon = CMD_BAR_BUTTON_ICON_DEFEND,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL,
		.fixedParam = 0,
		.icon = CMD_BAR_BUTTON_ICON_CANCEL,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton BUILD_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_building_select_button,
		.hotkeyIndex = KEY_B,
		.hotkey = "B",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_BUILD,
		.fixedParam = 0,
		.icon = CMD_BAR_BUTTON_ICON_BUILD,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton BUILD_ADVANCED_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_building_select_advanced_button,
		.hotkeyIndex = KEY_V,
		.hotkey = "V",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_BUILD_ADVANCED,
		.fixedParam = UNIT_STATE_MOVE,
		.icon = CMD_BAR_BUTTON_ICON_REPAIR,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton REPAIR_HARVEST_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_W,
		.hotkey = "W",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_REPAIR_HARVEST,
		.fixedParam = UNIT_STATE_MOVE,
		.icon = CMD_BAR_BUTTON_ICON_REPAIR_HARVEST,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_SELECT_BUILDING_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_build_select_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL,
		.fixedParam = 0,
		.icon = CMD_BAR_BUTTON_ICON_CANCEL,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_PLACE_BUILDING_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_build_place_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL,
		.fixedParam = 0,
		.icon = CMD_BAR_BUTTON_ICON_CANCEL,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_BUILDING_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_build_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL_BUILDING,
		.fixedParam = 0,
		.icon = CMD_BAR_BUTTON_ICON_CANCEL,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_WORKER_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_W,
		.hotkey = "W",
		.hoverTextId = GAME_TEXT_ID_TRAIN_WORKER,
		.fixedParam = UNIT_TYPE_WORKER,
		.icon = CMD_BAR_BUTTON_ICON_TRAIN_WORKER,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_SOLDIER_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_S,
		.hotkey = "S",
		.hoverTextId = GAME_TEXT_ID_TRAIN_SOLDIER,
		.fixedParam = UNIT_TYPE_SOLDIER,
		.icon = CMD_BAR_BUTTON_ICON_TRAIN_SOLDIER,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_ARCHER_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_A,
		.hotkey = "A",
		.hoverTextId = GAME_TEXT_ID_TRAIN_ARCHER,
		.fixedParam = UNIT_TYPE_ARCHER,
		.icon = CMD_BAR_BUTTON_ICON_TRAIN_ARCHER,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_KNIGHT_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_K,
		.hotkey = "K",
		.hoverTextId = GAME_TEXT_ID_TRAIN_KNIGHT,
		.fixedParam = UNIT_TYPE_KNIGHT,
		.icon = CMD_BAR_BUTTON_ICON_TRAIN_KNIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_MAGE_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_M,
		.hotkey = "M",
		.hoverTextId = GAME_TEXT_ID_TRAIN_MAGE,
		.fixedParam = UNIT_TYPE_MAGE,
		.icon = CMD_BAR_BUTTON_ICON_TRAIN_MAGE,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CITY_HALL_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_C,
		.hotkey = "C",
		.hoverTextId = GAME_TEXT_ID_BUILD_CITY_HALL,
		.fixedParam = UNIT_TYPE_CITY_HALL,
		.icon = CMD_BAR_BUTTON_ICON_BUILD_CITY_HALL,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton FARM_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_F,
		.hotkey = "F",
		.hoverTextId = GAME_TEXT_ID_BUILD_FARM,
		.fixedParam = UNIT_TYPE_FARM,
		.icon = CMD_BAR_BUTTON_ICON_BUILD_FARM,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton BARRACKS_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_B,
		.hotkey = "B",
		.hoverTextId = GAME_TEXT_ID_BUILD_BARRACKS,
		.fixedParam = UNIT_TYPE_BARRACKS,
		.icon = CMD_BAR_BUTTON_ICON_BUILD_BARRACKS,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton BLACKSMITH_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_M,
		.hotkey = "M",
		.hoverTextId = GAME_TEXT_ID_BUILD_BLACKSMITH,
		.fixedParam = UNIT_TYPE_BLACKSMITH,
		.icon = CMD_BAR_BUTTON_ICON_BUILD_BLACKSMITH,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton STABLES_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_S,
		.hotkey = "S",
		.hoverTextId = GAME_TEXT_ID_BUILD_STABLES,
		.fixedParam = UNIT_TYPE_STABLES,
		.icon = CMD_BAR_BUTTON_ICON_BUILD_STABLES,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TOWER_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_T,
		.hotkey = "T",
		.hoverTextId = GAME_TEXT_ID_BUILD_TOWER,
		.fixedParam = UNIT_TYPE_TOWER,
		.icon = CMD_BAR_BUTTON_ICON_BUILD_TOWER,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton UPGRADE_SOLDIER_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_S,
		.hotkey = "S",
		.hoverTextId = GAME_TEXT_ID_RESEARCH_SOLDIER,
		.fixedParam = TRAINING_TYPE_UPGRADE_SOLDIER,
		.icon = CMD_BAR_BUTTON_ICON_UPGRADE_SOLDIER,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton UPGRADE_ARCHER_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_A,
		.hotkey = "A",
		.hoverTextId = GAME_TEXT_ID_RESEARCH_ARCHER,
		.fixedParam = TRAINING_TYPE_UPGRADE_ARCHER,
		.icon = CMD_BAR_BUTTON_ICON_UPGRADE_ARCHER,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton UPGRADE_KNIGHT_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_K,
		.hotkey = "K",
		.hoverTextId = GAME_TEXT_ID_RESEARCH_KNIGHT,
		.fixedParam = TRAINING_TYPE_UPGRADE_KNIGHT,
		.icon = CMD_BAR_BUTTON_ICON_UPGRADE_KNIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton UPGRADE_MAGE_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_CREATE,
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_G,
		.hotkey = "G",
		.hoverTextId = GAME_TEXT_ID_RESEARCH_MAGE,
		.fixedParam = TRAINING_TYPE_UPGRADE_MAGE,
		.icon = CMD_BAR_BUTTON_ICON_UPGRADE_MAGE,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_ALL_TRAIN_CMD_BUTTON = {
		.type = CMD_BAR_BTN_TYPE_ACTION,
		.isActive = TRUE,
		.action = handle_cancel_all_unit_training,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL_ALL_TRAINING,
		.fixedParam = 0,
		.icon = CMD_BAR_BUTTON_ICON_CANCEL,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static void game_cmd_bar_handle_building_buttons(GameContext *context, GameUnit *building) {
	if(building->state == BUILDING_STATE_COMPLETED) {
		switch (building->type) {
			case UNIT_TYPE_CITY_HALL:
				context->cmdBarButtons[buttonIndex++] = TRAIN_WORKER_CMD_BUTTON;
				break;
			case UNIT_TYPE_BARRACKS:
				context->cmdBarButtons[buttonIndex++] = TRAIN_SOLDIER_CMD_BUTTON;
				if (unitCount[UNIT_TYPE_BLACKSMITH] > 0) context->cmdBarButtons[buttonIndex++] = TRAIN_ARCHER_CMD_BUTTON;
				if (unitCount[UNIT_TYPE_STABLES] > 0) context->cmdBarButtons[buttonIndex++] = TRAIN_KNIGHT_CMD_BUTTON;
				break;
			case UNIT_TYPE_TOWER: {
				context->cmdBarButtons[buttonIndex++] = TRAIN_MAGE_CMD_BUTTON;
				GameUnitUpgrade *mageUpgrade = &context->upgrades[building->controller][UNIT_TYPE_MAGE];
				if(!mageUpgrade->enabled && mageUpgrade->researchable && !trainings[TRAINING_TYPE_UPGRADE_MAGE]) {
					context->cmdBarButtons[buttonIndex++] = UPGRADE_MAGE_CMD_BUTTON;
				}
				break;
			}
			case UNIT_TYPE_BLACKSMITH: {
				GameUnitUpgrade *soldierUpgrade = &context->upgrades[building->controller][UNIT_TYPE_SOLDIER];
				GameUnitUpgrade *archerUpgrade = &context->upgrades[building->controller][UNIT_TYPE_ARCHER];
				if(!soldierUpgrade->enabled && soldierUpgrade->researchable && !trainings[TRAINING_TYPE_UPGRADE_SOLDIER]) {
					context->cmdBarButtons[buttonIndex++] = UPGRADE_SOLDIER_CMD_BUTTON;
				}
				if(!archerUpgrade->enabled && archerUpgrade->researchable && !trainings[TRAINING_TYPE_UPGRADE_ARCHER]) {
					context->cmdBarButtons[buttonIndex++] = UPGRADE_ARCHER_CMD_BUTTON;
				}
				break;
			}
			case UNIT_TYPE_STABLES: {
				GameUnitUpgrade *knightUpgrade = &context->upgrades[building->controller][UNIT_TYPE_KNIGHT];
				if(!knightUpgrade->enabled && knightUpgrade->researchable && !trainings[TRAINING_TYPE_UPGRADE_KNIGHT]) {
					context->cmdBarButtons[buttonIndex++] = UPGRADE_KNIGHT_CMD_BUTTON;
				}
				break;
			}
			default:
				break;
		}
		if(building->typed.buildingData.isTraining || building->typed.buildingData.queueNextIndex > 0) {
			context->cmdBarButtons[buttonIndex++] = CANCEL_ALL_TRAIN_CMD_BUTTON;
		}
	}
	else {
		context->cmdBarButtons[buttonIndex++] = CANCEL_BUILDING_CMD_BUTTON;
	}
}

static void game_cmd_bar_add_common(CommandBarButton cmdBarButtons[CMD_BAR_BUTTONS]) {
	cmdBarButtons[buttonIndex++] = MOVE_CMD_BUTTON;
	cmdBarButtons[buttonIndex++] = STOP_CMD_BUTTON;
	cmdBarButtons[buttonIndex++] = ATTACK_CMD_BUTTON;
}

static void game_cmd_bar_handle_building_select_buttons(GameContext *context) {
	if(context->map.enableCityHall) context->cmdBarButtons[buttonIndex++] = CITY_HALL_CMD_BUTTON;
	if(context->map.enableFarm) context->cmdBarButtons[buttonIndex++] = FARM_CMD_BUTTON;
	if(context->map.enableBarracks) context->cmdBarButtons[buttonIndex++] = BARRACKS_CMD_BUTTON;
	context->cmdBarButtons[buttonIndex++] = CANCEL_SELECT_BUILDING_CMD_BUTTON;
}

static void game_cmd_bar_handle_building_select_advanced_buttons(GameContext *context) {
	if (context->map.enableBlacksmith && unitCount[UNIT_TYPE_BARRACKS]) context->cmdBarButtons[buttonIndex++] = BLACKSMITH_CMD_BUTTON;
	if (context->map.enableStables && unitCount[UNIT_TYPE_BLACKSMITH]) context->cmdBarButtons[buttonIndex++] = STABLES_CMD_BUTTON;
	if (context->map.enableTower && unitCount[UNIT_TYPE_STABLES]) context->cmdBarButtons[buttonIndex++] = TOWER_CMD_BUTTON;
	context->cmdBarButtons[buttonIndex++] = CANCEL_SELECT_BUILDING_CMD_BUTTON;
}

static void game_cmd_bar_queue_bar(RenderQueue *renderQueue, FONT *font, int value, int maxValue, const char *innerText,
								   int x, int y, uint8_t isHp) {
	int length = text_length(font, innerText);
	render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, font, innerText,
							 x + UNIT_SHEET_BAR_LENGTH / 2 - length / 2,
							 y + UNIT_SHEET_BAR_TEXT_Y_OFF, PAL_COLOR_WHITE, TRANSPARENT_INDEX);

	int barColor = PAL_COLOR_DARK_GREEN;
	if (isHp) {
		if (value < maxValue / HEALTH_BAR_HALF) barColor = PAL_COLOR_YELLOW;
		if (value < maxValue / HEALTH_BAR_QUARTER) barColor = PAL_COLOR_RED;
	}
	length = (value * UNIT_SHEET_BAR_LENGTH) / maxValue;
	render_queue_submit_rect_fill(renderQueue, UNIT_SHEET_Z_ORDER_BAR,
								  x, y,
								  x + length, y + UNIT_SHEET_BAR_HEIGHT,
								  barColor);
	render_queue_submit_rect_fill(renderQueue, UNIT_SHEET_Z_ORDER_BAR,
								  x + length, y,
								  x + UNIT_SHEET_BAR_LENGTH, y + UNIT_SHEET_BAR_HEIGHT,
								  PAL_COLOR_GRAY);
	render_queue_submit_rect(renderQueue, UNIT_SHEET_Z_ORDER_BAR_RECT,
							 x, y,
							 x + UNIT_SHEET_BAR_LENGTH, y + UNIT_SHEET_BAR_HEIGHT,
							 PAL_COLOR_WHITE);
}

static void game_render_unit_stats_background(GameContext *context, RenderQueue *renderQueue) {
	int backgroundColor = PAL_COLOR_PLAYER;
	if(game_selection_one_enemy_selected(context)) backgroundColor = PAL_COLOR_COMPUTER;

	render_queue_submit_rect_fill(renderQueue, UNIT_SHEET_Z_ORDER_BACKGROUND + 1, 
		UNIT_SHEET_BACK_X, UNIT_SHEET_BACK_Y,
		UNIT_SHEET_BACK_X + UNIT_SHEET_BACK_WIDTH, UNIT_SHEET_BACK_Y + UNIT_SHEET_BACK_HEIGHT,
		backgroundColor
	);
	render_queue_submit_rect(renderQueue, UNIT_SHEET_Z_ORDER_BACKGROUND + 2, 
		UNIT_SHEET_BACK_X, UNIT_SHEET_BACK_Y,
		UNIT_SHEET_BACK_X + UNIT_SHEET_BACK_WIDTH, UNIT_SHEET_BACK_Y + UNIT_SHEET_BACK_HEIGHT,
		PAL_COLOR_WHITE
	);
	render_queue_submit_line(renderQueue, UNIT_SHEET_Z_ORDER_BACKGROUND + 3, 
		UNIT_SHEET_BACK_X + UNIT_SHEET_BACK_WIDTH, UNIT_SHEET_BACK_Y,
		UNIT_SHEET_BACK_X + UNIT_SHEET_BACK_WIDTH, UNIT_SHEET_BACK_Y + UNIT_SHEET_BACK_HEIGHT,
		PAL_COLOR_GRAY
	);
	render_queue_submit_line(renderQueue, UNIT_SHEET_Z_ORDER_BACKGROUND + 3, 
		UNIT_SHEET_BACK_X, UNIT_SHEET_BACK_Y + UNIT_SHEET_BACK_HEIGHT,
		UNIT_SHEET_BACK_X + UNIT_SHEET_BACK_WIDTH, UNIT_SHEET_BACK_Y + UNIT_SHEET_BACK_HEIGHT,
		PAL_COLOR_GRAY
	);
}

static void game_cmd_bar_render_queue_submit_single_unit(GameContext *context, RenderQueue *renderQueue) {
	GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[0]);
	if (!unit) return;
	game_render_unit_stats_background(context, renderQueue);

	// Unit name
	const char* name;
	if(unit->isCustom) {
		name = unit->name;
	}
	else {
		name = text_get_by_id(GAME_TEXT_ID_UNIT_TYPE_WORKER + unit->type);
	}
	int namePostionX = UNIT_SHEET_COL_ONE_X;
	if(unit->isCustom || unit->isUpgraded) {
		RLE_SPRITE* customIcon;
		if(unit->isCustom) {
			customIcon = game_gfx_get_unit_icon(GAME_UNIT_ICON_CUSTOM);
		} else {
			customIcon = game_gfx_get_unit_icon(GAME_UNIT_ICON_FUTURE);
		}
		render_queue_submit_rle_sprite(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, customIcon,
				namePostionX, UNIT_SHEET_ROW_ONE_Y + 1);
		namePostionX += customIcon->w + 1;
	}
	render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, name,
								namePostionX, UNIT_SHEET_ROW_ONE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);

	// Unit HP bar
	separate_ints_in_string(unitHpText, HEALTH_SEPARATOR, unit->health, unit->maxHealth);
	game_cmd_bar_queue_bar(renderQueue, context->gameFont, unit->health, unit->maxHealth, unitHpText,
							UNIT_SHEET_HP_BAR_X, UNIT_SHEET_ROW_TWO_Y, TRUE);

	// If we are a building training show bar
	if (unit->isBuilding) {
		BuildingData *buildingData = &unit->typed.buildingData;
		if (buildingData->isTraining) {
			game_cmd_bar_queue_bar(renderQueue, context->gameFont,
									buildingData->currentTicks, buildingData->targetTicks,
									text_get_by_id(GAME_TEXT_ID_UNIT_TYPE_WORKER + buildingData->trainingType),
									UNIT_SHEET_TRAIN_BAR_X, UNIT_SHEET_ROW_THREE_Y, FALSE);
		}
		// Icons are on same area, show queue only if the unit has no range
		if(unit->maxAttackRange == 0) {
			RLE_SPRITE* queueIcon = game_gfx_get_unit_icon(GAME_UNIT_ICON_QUEUE);
			render_queue_submit_rle_sprite(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, queueIcon,
				UNIT_SHEET_COL_ONE_X, UNIT_SHEET_ROW_FOUR_Y);
			itoa(buildingData->queueNextIndex, unitsText, BASE_TEN_NUMBER);
			render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitsText,
										UNIT_SHEET_COL_ONE_X + queueIcon->w + 1, UNIT_SHEET_ROW_FOUR_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
		}
		// If building is in construction, show progress
		if(unit->state == BUILDING_STATE_CONSTRUCT) {
			game_cmd_bar_queue_bar(renderQueue, context->gameFont,
									buildingData->currentTicks, buildingData->targetTicks,
									text_get_by_id(GAME_TEXT_ID_BUILDING_COMPLETATION),
									UNIT_SHEET_TRAIN_BAR_X, UNIT_SHEET_ROW_THREE_Y, FALSE);
		}
	}

	if(unit->type == UNIT_TYPE_WORKER) {
		WorkerData *workerData = &unit->typed.workerData;
		if(workerData->carriedResourceType != RESOURCE_TYPE_NONE && workerData->carriedResourceQty > 0) {
			RLE_SPRITE* resourceIcon = game_gfx_get_unit_icon(GAME_ICON_GOLD + unit->typed.workerData.carriedResourceType);
			render_queue_submit_rle_sprite(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, resourceIcon,
						UNIT_SHEET_COL_TWO_X, UNIT_SHEET_ROW_THREE_Y + UNIT_SHEET_ROW_THREE_Y_TEXT_OFFSET);
			itoa(workerData->carriedResourceQty, unitsResQty, BASE_TEN_NUMBER);
			render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitsResQty,
										UNIT_SHEET_COL_TWO_X + resourceIcon->w + 1, UNIT_SHEET_ROW_THREE_Y + UNIT_SHEET_ROW_THREE_Y_TEXT_OFFSET, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
		}
	}

	// Unit data
	if (unit->minAttackRange > 0 || unit->maxAttackRange > 0) {
		RLE_SPRITE* rangeIcon = game_gfx_get_unit_icon(GAME_UNIT_ICON_RANGE);
		render_queue_submit_rle_sprite(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, rangeIcon,
			UNIT_SHEET_COL_ONE_X, UNIT_SHEET_ROW_FOUR_Y);
		separate_ints_in_string(unitAtRangeText, RANGE_SEPARATOR,unit->minAttackRange, unit->maxAttackRange);
		render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitAtRangeText,
									UNIT_SHEET_COL_ONE_X + rangeIcon->w + 1,
									UNIT_SHEET_ROW_FOUR_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
	}
	// Show damage if we have damage and are not a building in construction, damage is in the same spot as completed bar
	if ((unit->minDamage > 0 || unit->maxDamage > 0) && (!unit->isBuilding || unit->state != BUILDING_STATE_CONSTRUCT)) {
		RLE_SPRITE* attackIcon = game_gfx_get_unit_icon(GAME_UNIT_ICON_ATTACK);
		render_queue_submit_rle_sprite(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, attackIcon,
				UNIT_SHEET_COL_ONE_X, UNIT_SHEET_ROW_THREE_Y + UNIT_SHEET_ROW_THREE_Y_TEXT_OFFSET);
		separate_ints_in_string(unitDamageText, RANGE_SEPARATOR, unit->minDamage, unit->maxDamage);
		render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitDamageText,
									UNIT_SHEET_COL_ONE_X + attackIcon->w + 1,
									UNIT_SHEET_ROW_THREE_Y + UNIT_SHEET_ROW_THREE_Y_TEXT_OFFSET, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
	}
	// Armor
	RLE_SPRITE *armorIcon = game_gfx_get_unit_icon(GAME_UNIT_ICON_ARMOR);
	render_queue_submit_rle_sprite(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, armorIcon,
									UNIT_SHEET_COL_TWO_X, UNIT_SHEET_ROW_FOUR_Y);
	itoa(unit->armor, unitArmorText, BASE_TEN_NUMBER);
	render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitArmorText,
								UNIT_SHEET_COL_TWO_X + armorIcon->w + 1,
								UNIT_SHEET_ROW_FOUR_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
}

static void game_cmd_bar_render_queue_submit_multi_unit(GameContext *context, RenderQueue *renderQueue) {
	game_render_unit_stats_background(context, renderQueue);
	itoa(context->selectedUnitCount, unitsText, BASE_TEN_NUMBER);
	strcat(unitsText, text_get_by_id(GAME_TEXT_ID_SELECTED_UNITS));
	render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitsText,
							 UNIT_SHEET_COL_ONE_X, UNIT_SHEET_ROW_ONE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);

	int currentHealth = 0;
	int maxHealth = 0;
	for (int i = 0; i < context->selectedUnitCount; i++) {
		GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
		if (unit) {
			currentHealth += unit->health;
			maxHealth += unit->maxHealth;
		}
	}
	itoa((currentHealth * 100) / maxHealth, unitHpText, BASE_TEN_NUMBER);
	strcat(unitHpText, UNITS_PERCENT);
	game_cmd_bar_queue_bar(renderQueue, context->gameFont, currentHealth, maxHealth, unitHpText,
						   UNIT_SHEET_HP_BAR_X, UNIT_SHEET_ROW_TWO_Y, TRUE);
}

static void game_cmd_bar_render_queue_submit_btn_info(RenderQueue *renderQueue, FONT* font, CommandBarButton *button) {
	render_queue_submit_text_multicolor_shadow(
			renderQueue, UI_Z_ORDER + 505, font, text_get_by_id(button->hoverTextId),
			HOVER_MESSAGE_X, HOVER_MESSAGE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX, PAL_COLOR_BLACK);

	if (button->type == CMD_BAR_BTN_TYPE_CREATE) {
		TrainingTypeEnum unitType = (TrainingTypeEnum) button->fixedParam;
		UnitData *unitData = NULL;
		if (unitType < TRAINING_TYPE_UPGRADE_SOLDIER) unitData = game_unit_get_data((UnitTypeEnum) unitType);
		TrainingResourcesData *trainingResources = game_unit_get_training_resources(unitType);
		int totalResources = UNIT_USED_RESOURCES;
		int xOff = 0;
		if(unitType >= TRAINING_TYPE_UPGRADE_SOLDIER || (unitData && unitData->isBuilding)) {
			xOff = NO_UNIT_RESOURCE_X_OFF;
			totalResources = NO_UNIT_RESOURCES;
		}
		for (int i = 0; i < totalResources; i++) {
			Position pos = RESOURCE_LOCATIONS[i];
			render_queue_submit_sprite(
					renderQueue, RESOURCES_Z, game_gfx_get_icon(i), pos.x + xOff,
					pos.y, RND_FLAG_NORMAL);
			itoa(trainingResources->used[i], resourceBuffers[i], BASE_TEN_NUMBER);
			render_queue_submit_text_shadow(renderQueue, RESOURCES_Z,
									 font, resourceBuffers[i],
									 pos.x + RESOURCE_LOCATIONS_TEXT_X_OFF + xOff,
									 pos.y + RESOURCE_LOCATIONS_TEXT_Y_OFF, PAL_COLOR_WHITE,
									 TRANSPARENT_INDEX, PAL_COLOR_BLACK);
		}
	}
}

static void game_cmd_bar_handle_placing(GameContext *context) {
	if(context->buildPlacing.state == CMD_BAR_BUILD_STATE_PLACE &&
					context->mouseStatus.y > VIEWPORT_Y_MIN && context->mouseStatus.y < VIEWPORT_Y_MAX &&
					context->mouseStatus.x > VIEWPORT_X_MIN && context->mouseStatus.x < VIEWPORT_X_MAX) {
		context->buildPlacing.showBuilding = TRUE;
		UnitData* unitData = game_unit_get_data(context->buildPlacing.building);
		context->buildPlacing.size = unitData->tileSize;

		uint16_t x = game_spatial_get_board_x_position(context->xPosition, context->mouseStatus.x);
		uint16_t y = game_spatial_get_board_y_position(context->yPosition, context->mouseStatus.y);

		context->buildPlacing.canBuild = TRUE;
		for(int xOff = 0; xOff < context->buildPlacing.size; xOff++) {
			for(int yOff = 0; yOff < context->buildPlacing.size; yOff++) {
				if(context->walkabilityGrid[x + xOff][y + yOff] == WALKABILITY_FREE
					&& context->boardExploration[x + xOff][y + yOff] == BOARD_EXPLORED) {
					context->buildPlacing.placeResult[xOff][yOff] = TRUE;
				}
				else {
					context->buildPlacing.placeResult[xOff][yOff] = FALSE;
					context->buildPlacing.canBuild = FALSE;
				}
			}
		}
		context->buildPlacing.x = x;
		context->buildPlacing.y = y;
	}
	else {
		context->buildPlacing.showBuilding = FALSE;
	}
}

void game_cmd_bar_handle_buttons(GameContext *context) {
	// Clear all buttons
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		context->cmdBarButtons[i].isActive = FALSE;
	}
	buttonIndex = 0;
	for (int i = 0; i < UNIT_TYPE_NUMBER; i++) unitCount[i] = 0;
	for(int i = 0; i < TRAINING_TYPE_NUMBER; i++) trainings[i] = 0;

	if (context->selectedUnitCount == 0 || game_selection_one_enemy_selected(context)) {
		game_cmd_bar_clear_build_placing(&context->buildPlacing);
		return;
	}

	MouseCursorStateEnum cursorState = game_mouse_get_cursor_state();
	if (cursorState == MOUSE_CURSOR_ATTACK || cursorState == MOUSE_CURSOR_TARGET) {
		context->cmdBarButtons[buttonIndex++] = CANCEL_CMD_BUTTON;
	} else {
		if (context->selectedUnitCount == 1) {
			GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[0]);
			if (unit) {
				if (unit->isBuilding || unit->type == UNIT_TYPE_WORKER) {
					GameUnit **activeUnits = context->activeUnits;
					for (int i = 0; i < context->activeUnitCount; i++, activeUnits++) {
						GameUnit *currentUnit = *activeUnits;
						if (currentUnit && currentUnit->controller == UNIT_CONTROLLER_PLAYER &&
							(!currentUnit->isBuilding || currentUnit->state == BUILDING_STATE_COMPLETED)) {
							unitCount[currentUnit->type]++;
							if (currentUnit->isBuilding) {
								if (currentUnit->typed.buildingData.isTraining) {
									trainings[currentUnit->typed.buildingData.trainingType]++;
								}
								for (int j = 0; j < currentUnit->typed.buildingData.queueNextIndex; j++) {
									trainings[currentUnit->typed.buildingData.queue[j]]++;
								}
							}
						}
					}
				}
				if (unit->isBuilding) {
					game_cmd_bar_clear_build_placing(&context->buildPlacing);
					game_cmd_bar_handle_building_buttons(context, unit);
				} else {
					if (unit->type == UNIT_TYPE_WORKER) {
						switch (context->buildPlacing.state) {
							case CMD_BAR_BUILD_STATE_SELECT:
								game_cmd_bar_handle_building_select_buttons(context);
								break;
							case CMD_BAR_BUILD_STATE_SELECT_ADVANCED: {
								game_cmd_bar_handle_building_select_advanced_buttons(context);
								break;
							}
							case CMD_BAR_BUILD_STATE_PLACE:
								context->cmdBarButtons[buttonIndex++] = CANCEL_PLACE_BUILDING_CMD_BUTTON;
								break;
							case CMD_BAR_BUILD_STATE_NONE:
								game_cmd_bar_add_common(context->cmdBarButtons);
								if(context->map.enableCityHall || context->map.enableFarm || context->map.enableBarracks) {
									context->cmdBarButtons[buttonIndex++] = BUILD_CMD_BUTTON;
								}
								if(
									(context->map.enableBlacksmith && unitCount[UNIT_TYPE_BARRACKS]) ||
										(context->map.enableStables && unitCount[UNIT_TYPE_BLACKSMITH]) ||
										(context->map.enableTower && unitCount[UNIT_TYPE_STABLES])
								) {
									context->cmdBarButtons[buttonIndex++] = BUILD_ADVANCED_CMD_BUTTON;
								}
								context->cmdBarButtons[buttonIndex++] = REPAIR_HARVEST_CMD_BUTTON;
								break;
						}
					} else {
						game_cmd_bar_clear_build_placing(&context->buildPlacing);
						game_cmd_bar_add_common(context->cmdBarButtons);
						context->cmdBarButtons[buttonIndex++] = DEFEND_CMD_BUTTON;
					}
				}
			}
		} else {
			game_cmd_bar_clear_build_placing(&context->buildPlacing);
			game_cmd_bar_add_common(context->cmdBarButtons);
			uint8_t thereAreNonWorkers = FALSE;
			for (int i = 0; i < context->selectedUnitCount; i++) {
				GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
				if (unit && unit->type != UNIT_TYPE_WORKER) {
					thereAreNonWorkers = TRUE;
					break;
				}
			}
			if (thereAreNonWorkers > 0) {
				context->cmdBarButtons[buttonIndex++] = DEFEND_CMD_BUTTON;
			} else {
				context->cmdBarButtons[buttonIndex++] = REPAIR_HARVEST_CMD_BUTTON;
			}
		}
	}

	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		CommandBarButton *button = &context->cmdBarButtons[i];
		if (!button->isActive) continue;
		uint8_t buttonPress = FALSE;
		button->state = CMD_BAR_BTN_STATE_IDLE;

		if (keyboard_is_key_down(button->hotkeyIndex)) {
			buttonPress = keyboard_is_key_pressed(button->hotkeyIndex);
			button->state = CMD_BAR_BTN_STATE_DOWN;
		} else {
			if (keyboard_is_key_released(button->hotkeyIndex)) {
				button->state = CMD_BAR_BTN_STATE_RELEASED;
			} else {
				if (mouseX >= button->x && mouseX < button->x + CMD_BAR_BUTTON_WIDTH && mouseY >= button->y && mouseY < button->y + CMD_BAR_BUTTON_HEIGHT) {
					if (context->mouseStatus.isLeftDown) {
						buttonPress = context->mouseStatus.isLeftPressed;
						button->state = CMD_BAR_BTN_STATE_DOWN;
					} else {
						if (context->mouseStatus.isLeftReleased) {
							button->state = CMD_BAR_BTN_STATE_RELEASED;
						} else {
							button->state = CMD_BAR_BTN_STATE_HOVER;
						}
					}
				}
			}
		}

		if (buttonPress) game_snd_play_sound(GAME_SOUND_CLICK);
		if (button->state == CMD_BAR_BTN_STATE_RELEASED) button->action(context, button->fixedParam);
		// Only one button pressed
		if (button->state == CMD_BAR_BTN_STATE_DOWN) break;
	}
	game_cmd_bar_handle_placing(context);
}

void game_cmd_bar_render_queue_submit(GameContext *context, RenderQueue *renderQueue) {
	if (context->selectedUnitCount == 1) game_cmd_bar_render_queue_submit_single_unit(context, renderQueue);
	if (context->selectedUnitCount > 1) game_cmd_bar_render_queue_submit_multi_unit(context, renderQueue);

	uint8_t buttonPressed = FALSE;
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		CommandBarButton *button = &context->cmdBarButtons[i];
		if (!button->isActive) continue;
		if (button->state == CMD_BAR_BTN_STATE_DOWN) {
			buttonPressed = TRUE;
			break;
		}
	}

	// Render buttons
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		CommandBarButton *button = &context->cmdBarButtons[i];
		if (!button->isActive) continue;
		int xPos = button->x;
		int yPos = button->y;
		if (button->state == CMD_BAR_BTN_STATE_DOWN) {
			xPos++;
			yPos++;
			render_queue_submit_line(renderQueue,
									 CMD_BUTTON_DOWN_LINE_Z,
									 xPos, yPos,
									 xPos + CMD_BAR_BUTTON_LINE_WIDTH, yPos,
									 PAL_COLOR_GRAY);
			render_queue_submit_line(renderQueue,
									 CMD_BUTTON_DOWN_LINE_Z,
									 xPos, yPos,
									 xPos, yPos + CMD_BAR_BUTTON_LINE_HEIGTH,
									 PAL_COLOR_GRAY);
			render_queue_submit_rect(renderQueue,
									 CMD_BUTTON_DOWN_RECT_Z,
									 xPos, yPos,
									 xPos + CMD_BAR_BUTTON_WIDTH - 1, yPos + CMD_BAR_BUTTON_HEIGHT - 1,
									 PAL_COLOR_WHITE);
		}
		render_queue_submit_rle_sprite(
				renderQueue, CMD_BUTTON_Z, game_gfx_get_cmd_bar_button_icon(button->icon),
				xPos, yPos);
		render_queue_submit_text(
				renderQueue, CMD_BUTTON_Z + 1, context->gameFont, button->hotkey,
				xPos + 2, yPos + 2, PAL_COLOR_BLACK, TRANSPARENT_INDEX);
		if ((button->state == CMD_BAR_BTN_STATE_HOVER && !buttonPressed) ||
			button->state == CMD_BAR_BTN_STATE_DOWN || button->state == CMD_BAR_BTN_STATE_RELEASED) {
			render_queue_submit_rect(renderQueue,
									 CMD_BUTTON_HOVER_Z,
									 xPos - 1, yPos - 1,
									 xPos + CMD_BAR_BUTTON_WIDTH, yPos + CMD_BAR_BUTTON_HEIGHT,
									 PAL_COLOR_BLACK);
			game_cmd_bar_render_queue_submit_btn_info(renderQueue, context->gameFont, button);
		}
	}
	if(!buttonPressed && context->buildPlacing.state == CMD_BAR_BUILD_STATE_PLACE &&
					context->mouseStatus.y > VIEWPORT_Y_MIN && context->mouseStatus.y < VIEWPORT_Y_MAX &&
					context->mouseStatus.x > VIEWPORT_X_MIN && context->mouseStatus.x < VIEWPORT_X_MAX) {
		render_queue_submit_text_multicolor_shadow(
			renderQueue, UI_Z_ORDER + 505, context->gameFont, text_get_by_id(GAME_TEXT_ID_PUT_BUILDING),
			HOVER_MESSAGE_X, HOVER_MESSAGE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
	}
}

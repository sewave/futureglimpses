#include "cmd_bar.h"

#define CMD_BAR_BUTTON_INITIAL_X 3
#define CMD_BAR_BUTTON_INITIAL_Y 127

#define CMD_BAR_BUTTON_SEPARATION_WIDTH 34
#define CMD_BAR_BUTTON_SEPARATION_HEIGHT 24
#define CMD_BAR_BUTTON_LINE_WIDTH CMD_BAR_BUTTON_WIDTH - 2
#define CMD_BAR_BUTTON_LINE_HEIGTH CMD_BAR_BUTTON_HEIGHT - 2

#define CMD_BUTTON_Z UI_Z_ORDER + 505
#define CMD_BUTTON_HOVER_Z UI_Z_ORDER + 505
#define CMD_BUTTON_DOWN_RECT_Z UI_Z_ORDER + 506
#define CMD_BUTTON_DOWN_LINE_Z UI_Z_ORDER + 507

#define UNIT_SHEET_COL_ONE_X 5
#define UNIT_SHEET_ROW_ONE_Y 80
#define UNIT_SHEET_ROW_TWO_Y 90
#define UNIT_SHEET_ROW_THREE_Y 104
#define UNIT_SHEET_ROW_FOUR_Y 116
#define UNIT_SHEET_HP_BAR_X UNIT_SHEET_COL_ONE_X + 1
#define UNIT_SHEET_TRAIN_BAR_X UNIT_SHEET_COL_ONE_X + 1

#define UNIT_SHEET_BAR_LENGTH 58
#define UNIT_SHEET_BAR_HEIGHT 11
#define UNIT_SHEET_BAR_TEXT_Y_OFF 1
#define UNIT_SHEET_Z_ORDER_BACKGROUND UI_Z_ORDER + 505
#define UNIT_SHEET_Z_ORDER_BAR UI_Z_ORDER + 509
#define UNIT_SHEET_Z_ORDER_SHEET_TEXT UI_Z_ORDER + 510
#define UNIT_SHEET_Z_ORDER_BAR_RECT UI_Z_ORDER + 511

static uint8_t buttonIndex;
static uint8_t unitCount[UNIT_TYPE_NUMBER] = {0};
static char unitHpText[16];
static char unitDamageText[32];
static char unitAtRangeText[32];
static char unitsText[32];

static void handle_train_unit(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	if (context->selectedUnitCount == 1) {
		GameUnit *building = game_unit_get_by_id(context, context->selectedUnits[0]);
		if (building) building_add_to_train_queue(context, building, (UnitTypeEnum) fixedDat);
	}
}

static void handle_cancel_button(void *ctxVoid, uint8_t fixedDat) {
	game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
}

static void handle_building_select_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	context->buildPlacing.state = CMD_BAR_BUILD_STATE_SELECT;
}

static void handle_repair_button(void *ctxVoid, uint8_t fixedDat) {
	// TODO handle repair button
}

static void handle_harvest_button(void *ctxVoid, uint8_t fixedDat) {
	// TODO handle harvest button
}

static void handle_build_place_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	context->buildPlacing.state = CMD_BAR_BUILD_STATE_PLACE;
	context->buildPlacing.building = (UnitTypeEnum) fixedDat;
}

static void handle_build_select_cancel_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	context->buildPlacing.state = CMD_BAR_BUILD_STATE_NONE;
}

static void handle_build_place_cancel_button(void *ctxVoid, uint8_t fixedDat) {
	GameContext *context = (GameContext *) ctxVoid;
	context->buildPlacing.state = CMD_BAR_BUILD_STATE_SELECT;
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
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_M,
		.hotkey = "M",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_MOVE,
		.fixedParam = UNIT_STATE_MOVE,
		.sheetOffsetX = 0,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton STOP_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_S,
		.hotkey = "S",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_STOP,
		.fixedParam = UNIT_STATE_IDLE,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton ATTACK_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_A,
		.hotkey = "A",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_ATTACK,
		.fixedParam = UNIT_STATE_ATTACK,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 2,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton DEFEND_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_action_button,
		.hotkeyIndex = KEY_D,
		.hotkey = "D",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_DEFEND,
		.fixedParam = UNIT_STATE_DEFEND,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 3,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL,
		.fixedParam = 0,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 4,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_WORKER_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_W,
		.hotkey = "W",
		.hoverTextId = GAME_TEXT_ID_TRAIN_WORKER,
		.fixedParam = UNIT_TYPE_WORKER,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 0,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_SOLDIER_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_S,
		.hotkey = "S",
		.hoverTextId = GAME_TEXT_ID_TRAIN_SOLDIER,
		.fixedParam = UNIT_TYPE_SOLDIER,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 1,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_ARCHER_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_A,
		.hotkey = "A",
		.hoverTextId = GAME_TEXT_ID_TRAIN_ARCHER,
		.fixedParam = UNIT_TYPE_ARCHER,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 2,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_KNIGHT_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_K,
		.hotkey = "K",
		.hoverTextId = GAME_TEXT_ID_TRAIN_KNIGHT,
		.fixedParam = UNIT_TYPE_KNIGHT,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 3,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TRAIN_MAGE_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_train_unit,
		.hotkeyIndex = KEY_M,
		.hotkey = "M",
		.hoverTextId = GAME_TEXT_ID_TRAIN_MAGE,
		.fixedParam = UNIT_TYPE_MAGE,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 4,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton BUILD_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_building_select_button,
		.hotkeyIndex = KEY_B,
		.hotkey = "B",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_BUILD,
		.fixedParam = 0,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 5,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton REPAIR_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_repair_button,
		.hotkeyIndex = KEY_R,
		.hotkey = "R",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_REPAIR,
		.fixedParam = 0,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 6,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton HARVEST_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_harvest_button,
		.hotkeyIndex = KEY_H,
		.hotkey = "H",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_HARVEST,
		.fixedParam = 0,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 7,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_SELECT_BUILDING_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_build_select_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL,
		.fixedParam = 0,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 4,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_PLACE_BUILDING_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_build_place_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hotkey = "ESC",
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL,
		.fixedParam = 0,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 4,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton FARM_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_F,
		.hotkey = "F",
		.hoverTextId = GAME_TEXT_ID_BUILD_FARM,
		.fixedParam = UNIT_TYPE_FARM,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 5,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton BARRACKS_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_B,
		.hotkey = "B",
		.hoverTextId = GAME_TEXT_ID_BUILD_BARRACKS,
		.fixedParam = UNIT_TYPE_BARRACKS,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 6,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton BLACKSMITH_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_M,
		.hotkey = "M",
		.hoverTextId = GAME_TEXT_ID_BUILD_BLACKSMITH,
		.fixedParam = UNIT_TYPE_BLACKSMITH,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 7,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton STABLES_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_S,
		.hotkey = "S",
		.hoverTextId = GAME_TEXT_ID_BUILD_STABLES,
		.fixedParam = UNIT_TYPE_STABLES,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 8,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton TOWER_CMD_BUTTON = {
		.isActive = TRUE,
		.action = handle_build_place_button,
		.hotkeyIndex = KEY_T,
		.hotkey = "T",
		.hoverTextId = GAME_TEXT_ID_BUILD_TOWER,
		.fixedParam = UNIT_TYPE_TOWER,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 9,
		.sheetOffsetY = CMD_BAR_BUTTON_HEIGHT,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

static void game_cmd_bar_handle_building_buttons(GameContext *context, GameUnit *building) {
	switch (building->type) {
		case UNIT_TYPE_CITY_HALL:
			context->cmdBarButtons[buttonIndex++] = TRAIN_WORKER_CMD_BUTTON;
			break;
		case UNIT_TYPE_BARRACKS:
			context->cmdBarButtons[buttonIndex++] = TRAIN_SOLDIER_CMD_BUTTON;
			if (unitCount[UNIT_TYPE_BLACKSMITH] > 0) context->cmdBarButtons[buttonIndex++] = TRAIN_ARCHER_CMD_BUTTON;
			if (unitCount[UNIT_TYPE_STABLES] > 0) context->cmdBarButtons[buttonIndex++] = TRAIN_KNIGHT_CMD_BUTTON;
			break;
		case UNIT_TYPE_TOWER:
			context->cmdBarButtons[buttonIndex++] = TRAIN_MAGE_CMD_BUTTON;
			break;
		default:
			break;
	}
}

static void game_cmd_bar_add_common(CommandBarButton cmdBarButtons[CMD_BAR_BUTTONS]) {
	cmdBarButtons[buttonIndex++] = MOVE_CMD_BUTTON;
	cmdBarButtons[buttonIndex++] = STOP_CMD_BUTTON;
	cmdBarButtons[buttonIndex++] = ATTACK_CMD_BUTTON;
}

static void game_cmd_bar_handle_building_select_buttons(GameContext *context) {
	context->cmdBarButtons[buttonIndex++] = FARM_CMD_BUTTON;
	context->cmdBarButtons[buttonIndex++] = BARRACKS_CMD_BUTTON;
	if (unitCount[UNIT_TYPE_BARRACKS]) context->cmdBarButtons[buttonIndex++] = BLACKSMITH_CMD_BUTTON;
	if (unitCount[UNIT_TYPE_BLACKSMITH]) context->cmdBarButtons[buttonIndex++] = STABLES_CMD_BUTTON;
	if (unitCount[UNIT_TYPE_STABLES]) context->cmdBarButtons[buttonIndex++] = TOWER_CMD_BUTTON;
	context->cmdBarButtons[buttonIndex++] = CANCEL_SELECT_BUILDING_CMD_BUTTON;
}

void game_cmd_bar_handle_buttons(GameContext *context) {
	// Clear all buttons
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		context->cmdBarButtons[i].isActive = FALSE;
	}
	buttonIndex = 0;
	for (int i = 0; i < UNIT_TYPE_NUMBER; i++) {
		unitCount[i] = 0;
	}

	if (context->selectedUnitCount == 0) {
		context->buildPlacing.state = CMD_BAR_BUILD_STATE_NONE;
		return;
	}

	MouseCursorStateEnum cursorState = game_mouse_get_cursor_state();
	if (cursorState == MOUSE_CURSOR_ATTACK || cursorState == MOUSE_CURSOR_TARGET) {
		context->cmdBarButtons[buttonIndex++] = CANCEL_CMD_BUTTON;
	} else {
		if (context->selectedUnitCount == 1) {
			GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[0]);
			if (unit) {
				GameUnit **activeUnits = context->activeUnits;
				for (int i = 0; i < context->activeUnitCount; i++, activeUnits++) {
					GameUnit *unit = *activeUnits;
					if (unit && unit->controller == UNIT_CONTROLLER_PLAYER) unitCount[unit->type]++;
				}
				if (unit->isBuilding) {
					context->buildPlacing.state = CMD_BAR_BUILD_STATE_NONE;
					game_cmd_bar_handle_building_buttons(context, unit);
				} else {
					if (unit->type == UNIT_TYPE_WORKER) {
						switch (context->buildPlacing.state) {
							case CMD_BAR_BUILD_STATE_SELECT:
								game_cmd_bar_handle_building_select_buttons(context);
								break;
							case CMD_BAR_BUILD_STATE_PLACE:
								context->cmdBarButtons[buttonIndex++] = CANCEL_PLACE_BUILDING_CMD_BUTTON;
								break;
							case CMD_BAR_BUILD_STATE_NONE:
								game_cmd_bar_add_common(context->cmdBarButtons);
								context->cmdBarButtons[buttonIndex++] = BUILD_CMD_BUTTON;
								context->cmdBarButtons[buttonIndex++] = REPAIR_CMD_BUTTON;
								context->cmdBarButtons[buttonIndex++] = HARVEST_CMD_BUTTON;
								break;
						}
					} else {
						context->buildPlacing.state = CMD_BAR_BUILD_STATE_NONE;
						game_cmd_bar_add_common(context->cmdBarButtons);
						context->cmdBarButtons[buttonIndex++] = DEFEND_CMD_BUTTON;
					}
				}
			}
		} else {
			context->buildPlacing.state = CMD_BAR_BUILD_STATE_NONE;
			game_cmd_bar_add_common(context->cmdBarButtons);
			uint8_t therAreNonWorkers = FALSE;
			for (int i = 0; i < context->selectedUnitCount; i++) {
				GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[i]);
				if (unit && unit->type != UNIT_TYPE_WORKER) {
					therAreNonWorkers = TRUE;
					break;
				}
			}
			if (therAreNonWorkers > 0) {
				context->cmdBarButtons[buttonIndex++] = DEFEND_CMD_BUTTON;
			} else {
				context->cmdBarButtons[buttonIndex++] = REPAIR_CMD_BUTTON;
				context->cmdBarButtons[buttonIndex++] = HARVEST_CMD_BUTTON;
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
	}
}

static void game_cmd_bar_queue_bar(RenderQueue *renderQueue, FONT *font, int value, int maxValue, const char *innerText,
								   int x, int y, uint8_t isHp) {
	int textLength = text_length(font, innerText);
	render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, font, innerText,
							 x + UNIT_SHEET_BAR_LENGTH / 2 - textLength / 2,
							 y + UNIT_SHEET_BAR_TEXT_Y_OFF, PAL_COLOR_WHITE, TRANSPARENT_INDEX);

	int barColor = PAL_COLOR_DARK_GREEN;
	if (isHp) {
		if (value < maxValue / HEALTH_BAR_HALF) barColor = PAL_COLOR_YELLOW;
		if (value < maxValue / HEALTH_BAR_QUARTER) barColor = PAL_COLOR_RED;
	}
	textLength = (value * UNIT_SHEET_BAR_LENGTH) / maxValue;
	render_queue_submit_rect_fill(renderQueue, UNIT_SHEET_Z_ORDER_BAR,
								  x, y,
								  x + textLength, y + UNIT_SHEET_BAR_HEIGHT,
								  barColor);
	render_queue_submit_rect_fill(renderQueue, UNIT_SHEET_Z_ORDER_BAR,
								  x + textLength, y,
								  x + UNIT_SHEET_BAR_LENGTH, y + UNIT_SHEET_BAR_HEIGHT,
								  PAL_COLOR_GRAY);
	render_queue_submit_rect(renderQueue, UNIT_SHEET_Z_ORDER_BAR_RECT,
							 x, y,
							 x + UNIT_SHEET_BAR_LENGTH, y + UNIT_SHEET_BAR_HEIGHT,
							 PAL_COLOR_WHITE);
}

void game_cmd_bar_render_queue_submit(GameContext *context, RenderQueue *renderQueue) {
	if (context->selectedUnitCount == 1) {
		GameUnit *unit = game_unit_get_by_id(context, context->selectedUnits[0]);
		if (unit) {
			// TODO unit stats background

			// Unit name
			render_queue_submit_text(renderQueue, UI_Z_ORDER + 510, context->gameFont,
									 text_get_by_id(GAME_TEXT_ID_UNIT_TYPE_WORKER + unit->type),
									 UNIT_SHEET_COL_ONE_X, UNIT_SHEET_ROW_ONE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);

			// Unit HP bar
			snprintf(unitHpText, sizeof(unitHpText), text_get_by_id(GAME_TEXT_ID_UNIT_SHEET_HP), unit->health, unit->maxHealth);
			game_cmd_bar_queue_bar(renderQueue, context->gameFont, unit->health, unit->maxHealth, unitHpText,
								   UNIT_SHEET_HP_BAR_X, UNIT_SHEET_ROW_TWO_Y, TRUE);

			// If we are a building training show bar
			if (unit->isBuilding) {
				BuildingData *buildingData = &unit->typed.buildingData;
				if (buildingData->isTraining) {
					game_cmd_bar_queue_bar(renderQueue, context->gameFont,
										   buildingData->currentTrainTicks, buildingData->targetTrainTicks,
										   text_get_by_id(GAME_TEXT_ID_UNIT_TYPE_WORKER + buildingData->trainUnit),
										   UNIT_SHEET_TRAIN_BAR_X, UNIT_SHEET_ROW_THREE_Y, FALSE);
				}
				if (buildingData->queueNextIndex > 0) {
					snprintf(unitsText, sizeof(unitsText), text_get_by_id(GAME_TEXT_ID_IN_QUEUE), buildingData->queueNextIndex);
					render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitsText,
											 UNIT_SHEET_COL_ONE_X, UNIT_SHEET_ROW_FOUR_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
				}
			}

			// Unit data
			if (unit->minAttackRange > 0 || unit->maxAttackRange > 0) {
				snprintf(unitAtRangeText, sizeof(unitAtRangeText), text_get_by_id(GAME_TEXT_ID_UNIT_SHEET_RANGE), unit->minAttackRange, unit->maxAttackRange);
				render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitAtRangeText,
										 UNIT_SHEET_COL_ONE_X,
										 UNIT_SHEET_ROW_FOUR_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
			}

			if (unit->minDamage > 0 || unit->maxDamage > 0) {
				snprintf(unitDamageText, sizeof(unitDamageText), text_get_by_id(GAME_TEXT_ID_UNIT_SHEET_DAMAGE), unit->minDamage, unit->maxDamage);
				render_queue_submit_text(renderQueue, UNIT_SHEET_Z_ORDER_SHEET_TEXT, context->gameFont, unitDamageText,
										 UNIT_SHEET_COL_ONE_X,
										 UNIT_SHEET_ROW_THREE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
			}
		}
	}
	if (context->selectedUnitCount > 1) {
		snprintf(unitsText, sizeof(unitsText), text_get_by_id(GAME_TEXT_ID_SELECTED_UNITS), context->selectedUnitCount);
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
		snprintf(unitHpText, sizeof(unitHpText), text_get_by_id(GAME_TEXT_ID_UNIT_SHEET_HP_PERCENT), (currentHealth * 100) / maxHealth);
		game_cmd_bar_queue_bar(renderQueue, context->gameFont, currentHealth, maxHealth, unitHpText,
							   UNIT_SHEET_HP_BAR_X, UNIT_SHEET_ROW_TWO_Y, TRUE);
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
		render_queue_submit_masked_partial(
				renderQueue, CMD_BUTTON_Z, context->cmdBarButtonsGfx,
				button->sheetOffsetX, button->sheetOffsetY,
				xPos, yPos,
				CMD_BAR_BUTTON_WIDTH, CMD_BAR_BUTTON_HEIGHT);

		render_queue_submit_text(
				renderQueue, CMD_BUTTON_Z + 1, context->gameFont, button->hotkey,
				xPos + 2, yPos + 2, PAL_COLOR_BLACK, TRANSPARENT_INDEX);
		if (button->state != CMD_BAR_BTN_STATE_IDLE) {
			render_queue_submit_rect(renderQueue,
									 CMD_BUTTON_HOVER_Z,
									 xPos - 1, yPos - 1,
									 xPos + CMD_BAR_BUTTON_WIDTH, yPos + CMD_BAR_BUTTON_HEIGHT,
									 PAL_COLOR_BLACK);
			render_queue_submit_text_multicolor(
					renderQueue, UI_Z_ORDER + 505, context->gameFont, text_get_by_id(button->hoverTextId),
					HOVER_MESSAGE_X, HOVER_MESSAGE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
			// TODO draw resource costs
		}
	}
}

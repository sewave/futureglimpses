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

static void handle_cancel_button(void *ctxVoid, uint8_t fixedDat) {
    game_mouse_set_cursor_state(MOUSE_CURSOR_IDLE);
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
		.type = CMD_BAR_BTN_ACTION,
		.action = handle_action_button,
		.hotkeyIndex = KEY_M,
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_MOVE,
		.fixedParam = UNIT_STATE_MOVE,
		.sheetOffsetX = 0,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton STOP_CMD_BUTTON = {
		.type = CMD_BAR_BTN_ACTION,
		.action = handle_action_button,
		.hotkeyIndex = KEY_S,
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_STOP,
		.fixedParam = UNIT_STATE_IDLE,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton ATTACK_CMD_BUTTON = {
		.type = CMD_BAR_BTN_ACTION,
		.action = handle_action_button,
		.hotkeyIndex = KEY_A,
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_ATTACK,
		.fixedParam = UNIT_STATE_ATTACK,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 2,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton DEFEND_CMD_BUTTON = {
		.type = CMD_BAR_BTN_ACTION,
		.action = handle_action_button,
		.hotkeyIndex = KEY_D,
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_DEFEND,
		.fixedParam = UNIT_STATE_DEFEND,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 3,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT,
		.state = CMD_BAR_BTN_STATE_IDLE};

static const CommandBarButton CANCEL_CMD_BUTTON = {
		.type = CMD_BAR_BTN_ACTION,
		.action = handle_cancel_button,
		.hotkeyIndex = KEY_ESC,
		.hoverTextId = GAME_TEXT_ID_CMD_BAR_CANCEL,
		.fixedParam = 0,
		.sheetOffsetX = CMD_BAR_BUTTON_WIDTH * 4,
		.sheetOffsetY = 0,
		.x = CMD_BAR_BUTTON_INITIAL_X + CMD_BAR_BUTTON_SEPARATION_WIDTH,
		.y = CMD_BAR_BUTTON_INITIAL_Y + CMD_BAR_BUTTON_SEPARATION_HEIGHT * 2,
		.state = CMD_BAR_BTN_STATE_IDLE};

void game_cmd_bar_handle_buttons(GameContext *context) {
	// Clear all buttons
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		context->cmdBarButtons[i].type = CMD_BAR_BTN_NONE;
	}

	if (context->selectedUnitCount == 0) return;

	// TODO other buttons
	// TODO configure buttons per scenario
    MouseCursorStateEnum cursorState = game_mouse_get_cursor_state();
    if(cursorState == MOUSE_CURSOR_ATTACK || cursorState == MOUSE_CURSOR_TARGET) {
        context->cmdBarButtons[0] = CANCEL_CMD_BUTTON;
    }
    else {
        context->cmdBarButtons[0] = MOVE_CMD_BUTTON;
        context->cmdBarButtons[1] = STOP_CMD_BUTTON;
        context->cmdBarButtons[2] = ATTACK_CMD_BUTTON;
        context->cmdBarButtons[3] = DEFEND_CMD_BUTTON;
    }

	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		CommandBarButton *button = &context->cmdBarButtons[i];
        if(button->type == CMD_BAR_BTN_NONE) continue;
		button->state = CMD_BAR_BTN_STATE_IDLE;
		// Check hotkeys
		if (keyboard_is_key_down(button->hotkeyIndex)) {
			button->state = CMD_BAR_BTN_STATE_DOWN;
		} else {
			if (keyboard_is_key_released(button->hotkeyIndex)) {
				button->state = CMD_BAR_BTN_STATE_RELEASED;
			}
		}

		if (button->state == CMD_BAR_BTN_STATE_IDLE && mouseX >= button->x && mouseX < button->x + CMD_BAR_BUTTON_WIDTH && mouseY >= button->y && mouseY < button->y + CMD_BAR_BUTTON_HEIGHT) {
			if (context->mouseStatus.isLeftDown || keyboard_is_key_down(button->hotkeyIndex)) {
				button->state = CMD_BAR_BTN_STATE_DOWN;
			} else {
				if (context->mouseStatus.isLeftReleased || keyboard_is_key_released(button->hotkeyIndex)) {
					button->state = CMD_BAR_BTN_STATE_RELEASED;
				} else {
					button->state = CMD_BAR_BTN_STATE_HOVER;
				}
			}
		}

		if (button->state == CMD_BAR_BTN_STATE_RELEASED) button->action(context, button->fixedParam);
	}
}

void game_cmd_bar_render_queue_submit(GameContext *context, RenderQueue *renderQueue) {
	for (int i = 0; i < CMD_BAR_BUTTONS; i++) {
		CommandBarButton *button = &context->cmdBarButtons[i];
        if(button->type == CMD_BAR_BTN_NONE) continue;
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
		if (button->state == CMD_BAR_BTN_STATE_HOVER) {
			render_queue_submit_rect(renderQueue,
									 CMD_BUTTON_HOVER_Z,
									 xPos - 1, yPos - 1,
									 xPos + CMD_BAR_BUTTON_WIDTH, yPos + CMD_BAR_BUTTON_HEIGHT,
									 PAL_COLOR_BLACK);
			render_queue_submit_text_multicolor(
					renderQueue, UI_Z_ORDER + 505, context->gameFont, text_get_by_id(button->hoverTextId),
					HOVER_MESSAGE_X, HOVER_MESSAGE_Y, PAL_COLOR_WHITE, TRANSPARENT_INDEX);
		}
	}
}

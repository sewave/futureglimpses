#include "gui.h"

#define GUI_Z_ORDER UI_Z_ORDER + 100

#define GUI_CHECK_WIDTH 8
#define GUI_CHECK_HEIGHT 8
#define GUI_CHECK_TEXT_X_OFF 10

#define GUI_BAR_WIDTH 128

#define GUI_BUTTON_TEXT_Y_OFFSET 4
#define GUI_BUTTON_SELECT_COLOR PAL_COLOR_GREEN
#define GUI_BUTTON_COLOR PAL_COLOR_DARK_GREEN

#define GUI_BUTTON_HOVER_COLOR PAL_COLOR_YELLOW
#define GUI_BUTTON_UP_WALL_COLOR PAL_COLOR_GRAY
#define GUI_BUTTON_DOWN_WALL_COLOR PAL_COLOR_DARK_GRAY
#define GUI_BUTTON_INTRA_WALL_COLOR PAL_COLOR_WHITE
#define GUI_BUTTON_INSIDE_WALL_COLOR PAL_COLOR_GRAY

#define GUI_OPTION_X_OFFSET 24
#define GUI_OPTION_Y_OFFSET 12

#define GUI_OPTION_VALUE_WIDTH 8
#define GUI_OPTION_VALUE_HEIGHT 8
#define GUI_OPTION_VALUE_HEIGHT_SEPARATION 12
#define GUI_OPTION_NO_VALUE -1

static const char * COLOR_CHANGE_STRING = "^000^000";

static int8_t game_gui_mouse_in_element_option(GameContext *context, GuiElement *element) {
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	int x = element->x + GUI_OPTION_X_OFFSET;
	int y = element->y + GUI_OPTION_Y_OFFSET;
	GuiOption *options = &element->typed.option;
	GuiOptionValue *optionValue = options->optionValues;
	for (int i = 0; i < options->optionValuesNumber; i++, optionValue++) {
		if (mouseX >= x && mouseX <= x + GUI_OPTION_VALUE_WIDTH &&
			mouseY >= y && mouseY <= y + GUI_OPTION_VALUE_HEIGHT) return i;
		y += GUI_OPTION_VALUE_HEIGHT_SEPARATION;
	}
	return GUI_OPTION_NO_VALUE;
}

static uint8_t game_gui_mouse_in_element(GameContext *context, GuiElement *element) {
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	uint8_t inElement = FALSE;
	switch (element->type) {
		case GUI_ELEMENT_BUTTON: {
			inElement = mouseX >= element->x && mouseX <= element->x + element->typed.button.size.width &&
						mouseY >= element->y && mouseY <= element->y + element->typed.button.size.height;
			break;
		}
		case GUI_ELEMENT_CHECK: {
			inElement = mouseX >= element->x && mouseX <= element->x + GUI_CHECK_WIDTH &&
						mouseY >= element->y && mouseY <= element->y + GUI_CHECK_HEIGHT;
			break;
		}
		case GUI_ELEMENT_OPTION: {
			inElement = game_gui_mouse_in_element_option(context, element) != GUI_OPTION_NO_VALUE;
			break;
		}
		case GUI_ELEMENT_BAR: {
			// TODO do mouse check based on static UI positioning
			break;
		}
		default:
			break;
	}
	return inElement;
}

void game_gui_handle(GameContext *context, GuiScreen* guiScreen) {
	GuiElement *element = guiScreen->elements;
	for (uint8_t i = 0; i < guiScreen->elementsCount; i++, element++) {
        if ((element->hotkey && keyboard_is_key_pressed(element->hotkey)) ||
            (context->mouseStatus.isLeftPressed && game_gui_mouse_in_element(context, element)) ||
            element->type == GUI_ELEMENT_OPTION
        ) {
			uint8_t doSound = FALSE;
			if (element->type == GUI_ELEMENT_OPTION) {
				GuiOption *options = &element->typed.option;
				GuiOptionValue *optionValueSearch = options->optionValues;
				int8_t mouseOption = game_gui_mouse_in_element_option(context, element);
				for (int i = 0; i < options->optionValuesNumber; i++, optionValueSearch++) {
					if ((optionValueSearch->hotkey && keyboard_is_key_pressed(optionValueSearch->hotkey)) ||
							(context->mouseStatus.isLeftPressed && mouseOption == i)) doSound = TRUE;
				}
			} else {
				doSound = TRUE;
			}
			if(doSound) game_snd_play_sound(GAME_SOUND_CLICK);
        }

		if ((element->hotkey && keyboard_is_key_released(element->hotkey)) ||
            (context->mouseStatus.isLeftReleased && game_gui_mouse_in_element(context, element)) ||
            element->type == GUI_ELEMENT_OPTION) {
			switch (element->type) {
				case GUI_ELEMENT_BUTTON: {
					element->typed.button.action(context);
					return;
				}
				case GUI_ELEMENT_CHECK: {
					uint8_t current = element->typed.check.getValue(context);
					element->typed.check.setValue(context, !current);
					return;
				}
				case GUI_ELEMENT_OPTION: {
					GuiOption *options = &element->typed.option;
					int8_t mouseOption = game_gui_mouse_in_element_option(context, element);
					GuiOptionValue *optionValue = NULL;
					if (mouseOption != GUI_OPTION_NO_VALUE) {
						optionValue = &options->optionValues[mouseOption];
					} else {
						GuiOptionValue *optionValueSearch = options->optionValues;
						for (int i = 0; i < options->optionValuesNumber; i++, optionValueSearch++) {
							if (keyboard_is_key_released(optionValueSearch->hotkey)) optionValue = optionValueSearch;
						}
					}
					if (optionValue) {
                        options->setValue(context, optionValue->value);
                        return;
                    }
					break;
				}
				case GUI_ELEMENT_BAR: {
                    GuiBar* bar = &element->typed.bar;
                    uint8_t leftPressed = FALSE;
                    uint8_t rightPressed = FALSE;
					// TODO button can click left side, right side or central bar
                    uint8_t value = bar->getValue(context);
                    uint8_t valueInc = bar->valueInc;
                    if(leftPressed) {                        
                        uint8_t minValue = bar->getMinValue(context);
                        if(value - minValue < valueInc) value = 0; else value -= valueInc;
                    }
                    else {
                        if(rightPressed) {
                            uint8_t maxValue = bar->getMaxValue(context);
                            if(value + valueInc > maxValue) value = maxValue; else value += valueInc;
                        }
                        else {
                            // TODO BAR PRESS, select value based on position
                        }
                    }
                    bar->setValue(context, value);
					break;
				}
				default:
					break;
			}
		}
	}
}

void game_gui_render_queue_submit(GameContext *context, RenderQueue *renderQueue, GuiScreen* guiScreen) {
	int colorChangeLength = text_length(context->gameFont, COLOR_CHANGE_STRING);
	GuiElement *element = guiScreen->elements;
	for (uint8_t i = 0; i < guiScreen->elementsCount; i++, element++) {
		int z = element->z;
		switch (element->type) {
			case GUI_ELEMENT_IMAGE: {
				render_queue_submit_solid(renderQueue, z, *element->typed.image.bitmap, element->x, element->y);
				break;
			}
			case GUI_ELEMENT_TEXT: {
				int x = element->x;
				if (element->typed.text.maxX > 0) {
					const char *text = text_get_by_id(element->textId);
					int textWidth = text_length(context->gameFont, text);
					x = element->x + (element->typed.text.maxX - element->x) / 2 - textWidth / 2;
				}
				render_queue_submit_text(renderQueue, z, context->gameFont, text_get_by_id(element->textId),
										 x, element->y, element->textColor, element->textBackground);
				break;
			}
			case GUI_ELEMENT_BUTTON: {
				int width = element->typed.button.size.width;
				int height = element->typed.button.size.height;

				const char *text = text_get_by_id(element->textId);
				int textX = element->x + element->typed.button.size.width / 2 - (text_length(context->gameFont, text) - colorChangeLength) / 2;
				render_queue_submit_text_multicolor(renderQueue, z + 1, context->gameFont, text,
										 textX, element->y + GUI_BUTTON_TEXT_Y_OFFSET, element->textColor, element->textBackground);
                int buttonColor = GUI_BUTTON_COLOR;
				if (game_gui_mouse_in_element(context, element)) {
					render_queue_submit_rect(renderQueue, z + 1, element->x, element->y,
											 element->x + width - 1, element->y + height - 1, GUI_BUTTON_HOVER_COLOR);
                    if(context->mouseStatus.isLeftDown) buttonColor = GUI_BUTTON_SELECT_COLOR;
				} else {
					// TODO render line walls

                    if(keyboard_is_key_down(element->hotkey)) buttonColor = GUI_BUTTON_SELECT_COLOR;
				}
                render_queue_submit_rect_fill(renderQueue, z, element->x, element->y,
											  element->x + width - 1, element->y + height - 1, buttonColor);

				// TODO render button
				render_queue_submit_rect(renderQueue, z + 1, element->x + 1, element->y + 1,
										 element->x + width - 2, element->y + height - 2, GUI_BUTTON_INTRA_WALL_COLOR);
                render_queue_submit_rect(renderQueue, z + 1, element->x + 2, element->y + 2,
                            element->x + width - 3, element->y + height - 3, GUI_BUTTON_INSIDE_WALL_COLOR);
				break;
			}
			case GUI_ELEMENT_CHECK: {
				GameIconEnum checkIcon = element->typed.check.getValue(context) ? GAME_ICON_CHECK_ON : GAME_ICON_CHECK_OFF;
				BITMAP *checkImage = game_gfx_get_icon(checkIcon);
				render_queue_submit_sprite(renderQueue, z, checkImage, element->x, element->y, RND_FLAG_NORMAL);
				render_queue_submit_text(renderQueue, z, context->gameFont, text_get_by_id(element->textId),
										 element->x + GUI_CHECK_TEXT_X_OFF, element->y, element->textColor, element->textBackground);
				break;
			}
			case GUI_ELEMENT_OPTION: {
				// Render all options
				// Render text

				// Render options with text, selected enabled

				uint8_t value = element->typed.option.getValue(context);
				GuiOptionValue *opt = &element->typed.option.optionValues[value];
				render_queue_submit_text(renderQueue, z, context->gameFont, text_get_by_id(opt->textId),
										 element->x, element->y, element->textColor, element->textBackground);
				break;
			}
			case GUI_ELEMENT_BAR: {
				// TODO Draw dec, inc and bar

				// TODO draw ball in position

				// Render progress bar
				break;
			}
		}
	}
}

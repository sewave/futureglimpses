#include "game/gui/gui.h"
#include "game/video/gfx.h"
#include "game/video/game_video.h"
#include "game/sound/game_sound.h"

#define GUI_Z_ORDER UI_Z_ORDER + 100

#define GUI_CHECK_WIDTH 8
#define GUI_CHECK_HEIGHT 8
#define GUI_CHECK_TEXT_X_OFF 10

#define GUI_BUTTON_TEXT_Y_OFFSET 4
#define GUI_BUTTON_FIT_TEXT_Y_OFFSET 0
#define GUI_BUTTON_SELECT_COLOR PAL_COLOR_TURQUOISE
#define GUI_BUTTON_COLOR PAL_COLOR_DARK_TURQUOISE

#define GUI_BUTTON_HOVER_COLOR PAL_COLOR_YELLOW
#define GUI_BUTTON_UP_WALL_COLOR PAL_COLOR_GRAY
#define GUI_BUTTON_DOWN_WALL_COLOR PAL_COLOR_DARK_GRAY
#define GUI_BUTTON_INTRA_WALL_COLOR PAL_COLOR_WHITE
#define GUI_BUTTON_INSIDE_WALL_COLOR PAL_COLOR_GRAY

#define GUI_OPTION_X_OFFSET 12
#define GUI_OPTION_Y_OFFSET 12
#define GUI_OPTION_TEXT_X_OFF 10

#define GUI_OPTION_VALUE_WIDTH 8
#define GUI_OPTION_VALUE_HEIGHT 8
#define GUI_OPTION_VALUE_HEIGHT_SEPARATION 12
#define GUI_OPTION_NO_VALUE -1

#define GUI_BAR_X_OFFSET -2
#define GUI_BAR_Y_OFFSET 12
#define GUI_BAR_WIDTH 128
#define GUI_BAR_ARROW_WIDTH 8
#define GUI_BAR_LANE_WIDTH (GUI_BAR_WIDTH - GUI_BAR_ARROW_WIDTH)

#define GUI_VERTICAL_BAR_X_OFFSET 0
#define GUI_VERTICAL_BAR_Y_OFFSET 0
#define GUI_VERTICAL_BAR_HEIGHT 104
#define GUI_VERTICAL_BAR_ARROW_HEIGHT 8
#define GUI_VERTICAL_BAR_LANE_HEIGHT (GUI_VERTICAL_BAR_HEIGHT - GUI_VERTICAL_BAR_ARROW_HEIGHT)

#define TEXT_ROWS_ICON_OFFSET 3

static const char * COLOR_CHANGE_STRING = "^000^000";

typedef enum {
	GUI_BAR_LEFT_BUTTON,
	GUI_BAR_RIGHT_BUTTON,
	GUI_BAR_CENTER,
	GUI_BAR_NONE,
} GuiBarPosition;

typedef enum {
	GUI_VERTICAL_BAR_UP_BUTTON,
	GUI_VERTICAL_BAR_DOWN_BUTTON,
	GUI_VERTICAL_BAR_CENTER,
	GUI_VERTICAL_BAR_NONE,
} GuiVerticalBarPosition;

/**
 * Checks whether the mouse is inside the bar element.
 */
static GuiBarPosition game_gui_mouse_in_bar(GameContext *context, GuiElement *element) {
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	
	BITMAP* barLeft = game_gfx_get_icon(GAME_ICON_BAR_LEFT_ON);
	BITMAP* barRight = game_gfx_get_icon(GAME_ICON_BAR_RIGHT_ON);
	if(mouseX >= element->x + GUI_BAR_X_OFFSET &&
	   mouseX < element->x + GUI_BAR_X_OFFSET + barLeft->w &&
	   mouseY >= element->y + GUI_BAR_Y_OFFSET &&
	   mouseY <= element->y + GUI_BAR_Y_OFFSET + barLeft->h) {
		return GUI_BAR_LEFT_BUTTON;
	}
	else if(mouseX >= element->x + GUI_BAR_X_OFFSET + GUI_BAR_WIDTH + barLeft->w &&
			mouseX < element->x + GUI_BAR_X_OFFSET + GUI_BAR_WIDTH  + barLeft->w + barRight->w &&
			mouseY >= element->y + GUI_BAR_Y_OFFSET &&
			mouseY <= element->y + GUI_BAR_Y_OFFSET + barRight->h) {
		return GUI_BAR_RIGHT_BUTTON;
	}
	else if(mouseX >= element->x + GUI_BAR_X_OFFSET + barLeft->w &&
			mouseX < element->x + GUI_BAR_X_OFFSET + GUI_BAR_WIDTH + barLeft->w &&
			mouseY >= element->y + GUI_BAR_Y_OFFSET &&
			mouseY <= element->y + GUI_BAR_Y_OFFSET + GUI_OPTION_VALUE_HEIGHT) {
		return GUI_BAR_CENTER;
	}
	return GUI_BAR_NONE;
}

/**
 * Checks whether the mouse is inside the vertical bar element.
 */
static GuiVerticalBarPosition game_gui_mouse_in_vertical_bar(GameContext *context, GuiElement *element) {
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;

	BITMAP *barUp = game_gfx_get_icon(GAME_ICON_BAR_LEFT_ON);
	BITMAP *barDown = game_gfx_get_icon(GAME_ICON_BAR_RIGHT_ON);
	if (mouseX >= element->x + GUI_VERTICAL_BAR_X_OFFSET &&
		mouseX < element->x + GUI_VERTICAL_BAR_X_OFFSET + barUp->w &&
		mouseY >= element->y + GUI_VERTICAL_BAR_Y_OFFSET &&
		mouseY <= element->y + GUI_VERTICAL_BAR_Y_OFFSET + barUp->h) {
		return GUI_VERTICAL_BAR_UP_BUTTON;
	} else if (mouseX >= element->x + GUI_VERTICAL_BAR_X_OFFSET &&
			   mouseX < element->x + GUI_VERTICAL_BAR_X_OFFSET + barDown->w &&
			   mouseY >= element->y + GUI_VERTICAL_BAR_Y_OFFSET + GUI_VERTICAL_BAR_HEIGHT + barUp->h &&
			   mouseY <= element->y + GUI_VERTICAL_BAR_Y_OFFSET + GUI_VERTICAL_BAR_HEIGHT + barUp->h + barDown->h) {
		return GUI_VERTICAL_BAR_DOWN_BUTTON;
	} else if (mouseX >= element->x + GUI_VERTICAL_BAR_X_OFFSET &&
			   mouseX < element->x + GUI_VERTICAL_BAR_X_OFFSET + GUI_OPTION_VALUE_WIDTH &&
			   mouseY >= element->y + GUI_VERTICAL_BAR_Y_OFFSET + barUp->h &&
			   mouseY <= element->y + GUI_VERTICAL_BAR_Y_OFFSET + GUI_VERTICAL_BAR_HEIGHT + barUp->h) {
		return GUI_VERTICAL_BAR_CENTER;
	}
	return GUI_VERTICAL_BAR_NONE;
}

static int8_t game_gui_mouse_in_element_option(GameContext *context, GuiElement *element) {
	int colorChangeLength = text_length(context->gameFont, COLOR_CHANGE_STRING);
	int mouseX = context->mouseStatus.x;
	int mouseY = context->mouseStatus.y;
	int x = element->x + GUI_OPTION_X_OFFSET;
	int y = element->y + GUI_OPTION_Y_OFFSET;
	GuiOption *options = &element->typed.option;
	GuiOptionValue *optionValue = options->optionValues;
	for (int i = 0; i < options->optionValuesNumber; i++, optionValue++, y += GUI_OPTION_VALUE_HEIGHT_SEPARATION) {
		if (mouseX >= x && mouseX <= x + GUI_OPTION_VALUE_WIDTH - colorChangeLength + text_length(context->gameFont, text_get_by_id(optionValue->textId)) &&
			mouseY >= y && mouseY <= y + GUI_OPTION_VALUE_HEIGHT) return i;
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
			inElement = mouseX >= element->x + GUI_BAR_X_OFFSET && mouseX <= element->x + GUI_BAR_X_OFFSET + GUI_BAR_WIDTH + 2 * GUI_BAR_ARROW_WIDTH &&
						mouseY >= element->y + GUI_BAR_Y_OFFSET && mouseY <= element->y + GUI_BAR_Y_OFFSET + GUI_OPTION_VALUE_HEIGHT;
			break;
		}
		case GUI_ELEMENT_VERTICAL_BAR: {
			inElement = mouseX >= element->x + GUI_VERTICAL_BAR_X_OFFSET && mouseX <= element->x + GUI_VERTICAL_BAR_X_OFFSET + GUI_OPTION_VALUE_WIDTH &&
						mouseY >= element->y + GUI_VERTICAL_BAR_Y_OFFSET && mouseY <= element->y + GUI_VERTICAL_BAR_Y_OFFSET + GUI_VERTICAL_BAR_HEIGHT + 2 * GUI_VERTICAL_BAR_ARROW_HEIGHT;
			break;
		}
		case GUI_ELEMENT_CUSTOM_TEXT_ROWS: {
			inElement = mouseX >= element->x && mouseX <= element->x + element->typed.customTextRows.width &&
			mouseY >= element->y && mouseY <= element->y + (element->typed.customTextRows.numRows) * element->typed.customTextRows.ySeparation;
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
					if (mouseOption != GUI_OPTION_NO_VALUE && context->mouseStatus.isLeftReleased) {
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
					GuiBarPosition barPosition = game_gui_mouse_in_bar(context, element);
                    GuiBar* bar = &element->typed.bar;
                    uint8_t value = bar->getValue(context);
                    uint8_t valueInc = bar->valueInc;
					switch (barPosition) {
						case GUI_BAR_LEFT_BUTTON: {
							uint8_t minValue = bar->getMinValue(context);
							if(value - minValue < valueInc) value = 0; else value -= valueInc;
							break;
						}
						case GUI_BAR_RIGHT_BUTTON: {
							uint8_t maxValue = bar->getMaxValue(context);
							if(maxValue - valueInc < value) value = maxValue; else value += valueInc;
							break;
						}
						case GUI_BAR_CENTER: {
							int maxValue = element->typed.bar.getMaxValue(context);
							int barStartX = element->x + GUI_BAR_X_OFFSET + GUI_BAR_ARROW_WIDTH;
							int relativeX = context->mouseStatus.x - barStartX;
							if(relativeX < 0) relativeX = 0;
							if(relativeX > GUI_BAR_LANE_WIDTH) relativeX = GUI_BAR_LANE_WIDTH;
							value = (relativeX * maxValue) / GUI_BAR_LANE_WIDTH;
							break;
						}
						case GUI_BAR_NONE:
						default:
							break;
					}
                    bar->setValue(context, value);
					break;
				}
				case GUI_ELEMENT_VERTICAL_BAR: {
					GuiVerticalBarPosition vBarPosition = game_gui_mouse_in_vertical_bar(context, element);
					GuiVerticalBar *vBar = &element->typed.vBar;
					uint8_t value = vBar->getValue(context);
					uint8_t valueInc = vBar->valueInc;
					switch (vBarPosition) {
						case GUI_VERTICAL_BAR_UP_BUTTON: {
							uint8_t minValue = vBar->getMinValue(context);
							if (value - minValue < valueInc) value = 0;
							else
								value -= valueInc;
							break;
						}
						case GUI_VERTICAL_BAR_DOWN_BUTTON: {
							uint8_t maxValue = vBar->getMaxValue(context);
							if (maxValue - valueInc < value) value = maxValue;
							else
								value += valueInc;
							break;
						}
						case GUI_VERTICAL_BAR_CENTER: {
							int maxValue = element->typed.vBar.getMaxValue(context);
							int barStartY = element->y + GUI_VERTICAL_BAR_Y_OFFSET + GUI_VERTICAL_BAR_ARROW_HEIGHT;
							int relativeY = context->mouseStatus.y - barStartY;
							if (relativeY < 0) relativeY = 0;
							if (relativeY > GUI_VERTICAL_BAR_LANE_HEIGHT) relativeY = GUI_VERTICAL_BAR_LANE_HEIGHT;
							value = (relativeY * maxValue) / GUI_VERTICAL_BAR_LANE_HEIGHT;
							break;
						}
						case GUI_VERTICAL_BAR_NONE:
						default:
							break;
					}
					vBar->setValue(context, value);
					break;
				}
				case GUI_ELEMENT_CUSTOM_TEXT_ROWS: {
					if(element->typed.customTextRows.setSelectedValue) {
						uint8_t offset = element->typed.customTextRows.getOffsetValue(context);
						uint8_t selectedRow = (context->mouseStatus.y - element->y) / element->typed.customTextRows.ySeparation;
						element->typed.customTextRows.setSelectedValue(context, offset + selectedRow);
					}
				}
				default:
					break;
			}
		}
		if (context->mouseStatus.isLeftDoubleClick && game_gui_mouse_in_element(context, element)) {
			if(element->type == GUI_ELEMENT_CUSTOM_TEXT_ROWS) {
					if(element->typed.customTextRows.rowAction) element->typed.customTextRows.rowAction(context);
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
				const char *text = text_get_by_id(element->textId);
				if (element->typed.text.maxX > 0) {
					int textWidth = text_length(context->gameFont, text);
					x = element->x + (element->typed.text.maxX - element->x) / 2 - textWidth / 2;
				}
				if(element->typed.text.maxWidth > 0 && element->typed.text.maxHeight > 0) {
					render_queue_submit_enclosed_text_shadow(renderQueue, z, context->gameFont, text,
											 x, element->y, element->typed.text.maxWidth, element->typed.text.maxHeight,
											 element->textColor, element->textBackground, element->shadowTextColor);
				}
				else {
					render_queue_submit_text_shadow(renderQueue, z, context->gameFont, text_get_by_id(element->textId),
											 x, element->y, element->textColor, element->textBackground, element->shadowTextColor);
				}
				break;
			}
			case GUI_ELEMENT_CUSTOM_TEXT: {
				int x = element->x;
				if (element->typed.customText.maxX > 0) {
					const char *text = element->typed.customText.text(context);
					int textWidth = text_length(context->gameFont, text);
					x = element->x + (element->typed.customText.maxX - element->x) / 2 - textWidth / 2;
				}
				char* text = element->typed.customText.text(context);
				if(text) {
					if(element->typed.customText.maxWidth > 0 && element->typed.customText.maxHeight > 0) {
						render_queue_submit_enclosed_text_shadow(renderQueue, z, context->gameFont, text,
												x, element->y, element->typed.customText.maxWidth, element->typed.customText.maxHeight,
												element->textColor, element->textBackground, element->shadowTextColor);
					}
					else {
						render_queue_submit_text_shadow(renderQueue, z, context->gameFont, text,
												x, element->y, element->textColor, element->textBackground, element->shadowTextColor);
					}
				}
				break;
			}
			case GUI_ELEMENT_CUSTOM_TEXT_ROWS: {
				GuiCustomTextRows *customTextRows = &element->typed.customTextRows;
				uint8_t offset = customTextRows->getOffsetValue(context);
				uint8_t selectedValue = customTextRows->getSelectedValue(context);
				uint8_t maxRow = customTextRows->getMaxRow(context);
				uint8_t lastCustomRow = offset + customTextRows->numRows - 1;
				uint8_t maxIndex = min_val(maxRow, lastCustomRow);
				int currentY = element->y;

				for (int i = offset; i <= maxIndex; i++) {
					const char *text = customTextRows->getText(context, i);
					if (text == NULL) break;
					BITMAP* icon = customTextRows->getIcon(context, i);

					int x = element->x;

					if(icon != NULL) {
						render_queue_submit_sprite(renderQueue, z, icon, x, currentY, RND_FLAG_NORMAL);
						x += icon->w + TEXT_ROWS_ICON_OFFSET;
					}

					if (customTextRows->maxX > 0) {
						int textWidth = text_length(context->gameFont, text);
						x = element->x + (customTextRows->maxX - element->x) / 2 - textWidth / 2;
					}

					// Determine text color: use selectedTextColor if index matches selected
					int textColor = element->textColor;
					if (i == selectedValue) textColor = customTextRows->selectedTextColor;

					if (customTextRows->maxWidth > 0 && customTextRows->maxHeight > 0) {
						render_queue_submit_enclosed_text_shadow(renderQueue, z, context->gameFont, text,
																 x, currentY, customTextRows->maxWidth, customTextRows->maxHeight,
																 textColor, element->textBackground, element->shadowTextColor);
					} else {
						render_queue_submit_text_shadow(renderQueue, z, context->gameFont, text,
														x, currentY, textColor, element->textBackground, element->shadowTextColor);
					}

					currentY += customTextRows->ySeparation;
				}
				break;
			}
			case GUI_ELEMENT_BUTTON: {
				int width = element->typed.button.size.width;
				int height = element->typed.button.size.height;

				const char *text = text_get_by_id(element->textId);
				int yOffset = GUI_BUTTON_TEXT_Y_OFFSET;
				if (element->typed.button.fit) {
					yOffset = GUI_BUTTON_FIT_TEXT_Y_OFFSET;
				}
				int textX = element->x + element->typed.button.size.width / 2 - (text_length(context->gameFont, text) - colorChangeLength) / 2;
				render_queue_submit_text_multicolor_shadow(renderQueue, z + 1, context->gameFont, text,
										 textX, element->y + yOffset, element->textColor, element->textBackground, element->shadowTextColor);
                int buttonColor = GUI_BUTTON_COLOR;
				if (game_gui_mouse_in_element(context, element)) {
					render_queue_submit_rect(renderQueue, z + 1, element->x, element->y,
											 element->x + width - 1, element->y + height - 1, GUI_BUTTON_HOVER_COLOR);
                    if(context->mouseStatus.isLeftDown) buttonColor = GUI_BUTTON_SELECT_COLOR;
				} else {
					render_queue_submit_line(
						renderQueue, z + 1, element->x, element->y,
						element->x + width - 1, element->y, GUI_BUTTON_UP_WALL_COLOR
					);
					render_queue_submit_line(
						renderQueue, z + 1, element->x, element->y,
						element->x, element->y + height - 2, GUI_BUTTON_UP_WALL_COLOR
					);
					render_queue_submit_line(
						renderQueue, z + 1, element->x + width - 1, element->y + 1,
						element->x + width - 1,  element->y + height - 1, GUI_BUTTON_DOWN_WALL_COLOR
					);
					render_queue_submit_line(
						renderQueue, z + 1, element->x, element->y + height - 1,
						element->x + width - 1, element->y + height - 1, GUI_BUTTON_DOWN_WALL_COLOR
					);

                    if(keyboard_is_key_down(element->hotkey)) buttonColor = GUI_BUTTON_SELECT_COLOR;
				}
                render_queue_submit_rect_fill(renderQueue, z, element->x, element->y,
											  element->x + width - 1, element->y + height - 1, buttonColor);
				if(!element->typed.button.fit) {
					render_queue_submit_rect(renderQueue, z + 1, element->x + 1, element->y + 1,
						element->x + width - 2, element->y + height - 2, GUI_BUTTON_INTRA_WALL_COLOR);
                	render_queue_submit_rect(renderQueue, z + 1, element->x + 2, element->y + 2,
						element->x + width - 3, element->y + height - 3, GUI_BUTTON_INSIDE_WALL_COLOR);
				}
				break;
			}
			case GUI_ELEMENT_CHECK: {
				GameIconEnum checkIcon = element->typed.check.getValue(context) ? GAME_ICON_CHECK_ON : GAME_ICON_CHECK_OFF;
				BITMAP *checkImage = game_gfx_get_icon(checkIcon);
				render_queue_submit_sprite(renderQueue, z, checkImage, element->x, element->y, RND_FLAG_NORMAL);
				render_queue_submit_text_shadow(renderQueue, z, context->gameFont, text_get_by_id(element->textId),
										 element->x + GUI_CHECK_TEXT_X_OFF, element->y, element->textColor, element->textBackground, element->shadowTextColor);
				break;
			}
			case GUI_ELEMENT_OPTION: {
				render_queue_submit_text_shadow(renderQueue, z, context->gameFont, text_get_by_id(element->textId),
										 element->x, element->y, element->textColor, element->textBackground, element->shadowTextColor);
				uint8_t value = element->typed.option.getValue(context);				
				GuiOption *options = &element->typed.option;
				GuiOptionValue *optionValue = options->optionValues;
				BITMAP* optionOnImage = game_gfx_get_icon(GAME_ICON_OPTION_ON);
				BITMAP* optionOffImage = game_gfx_get_icon(GAME_ICON_OPTION_OFF);
				int y = element->y + GUI_OPTION_Y_OFFSET;
				int x = element->x + GUI_OPTION_X_OFFSET;
				for (int i = 0; i < options->optionValuesNumber; i++, optionValue++, y += GUI_OPTION_VALUE_HEIGHT_SEPARATION) {
					BITMAP* optionImage = (i == value) ? optionOnImage : optionOffImage;
					render_queue_submit_sprite(renderQueue, z, optionImage, x, y, RND_FLAG_NORMAL);
					render_queue_submit_text_multicolor_shadow(renderQueue, z, context->gameFont,
						text_get_by_id(optionValue->textId), x + GUI_OPTION_TEXT_X_OFF, y,
						optionValue->textColor, optionValue->textBackground, optionValue->shadowTextColor);
				}
				break;
			}
			case GUI_ELEMENT_BAR: {
				render_queue_submit_text_shadow(renderQueue, z, context->gameFont, text_get_by_id(element->textId),
										 element->x, element->y, element->textColor, element->textBackground, element->shadowTextColor);
				BITMAP* barLeftOff = game_gfx_get_icon(GAME_ICON_BAR_LEFT_OFF);
				BITMAP* barLeftOn = game_gfx_get_icon(GAME_ICON_BAR_LEFT_ON);
				BITMAP* barRightOff = game_gfx_get_icon(GAME_ICON_BAR_RIGHT_OFF);
				BITMAP* barRightOn = game_gfx_get_icon(GAME_ICON_BAR_RIGHT_ON);
				BITMAP* barImage = game_gfx_get_icon(GAME_ICON_BAR);
				BITMAP* barBall = game_gfx_get_icon(GAME_ICON_OPTION_ON);

				BITMAP* barLeft = barLeftOff;
				BITMAP* barRight = barRightOff;

				if(context->mouseStatus.isLeftDown) {
					GuiBarPosition barPosition = game_gui_mouse_in_bar(context, element);
					switch(barPosition) {
						case GUI_BAR_LEFT_BUTTON:
							barLeft = barLeftOn;
							break;
						case GUI_BAR_RIGHT_BUTTON:
							barRight = barRightOn;
							break;
						case GUI_BAR_CENTER:
						case GUI_BAR_NONE:
						default:
							break;
					}
				}
				
				render_queue_submit_sprite(renderQueue, z, barLeft,
					element->x + GUI_BAR_X_OFFSET, element->y + GUI_BAR_Y_OFFSET, RND_FLAG_NORMAL);
				render_queue_submit_sprite(renderQueue, z, barRight,
					element->x + GUI_BAR_X_OFFSET + GUI_BAR_WIDTH + barLeft->w, element->y + GUI_BAR_Y_OFFSET, RND_FLAG_NORMAL);

				// Render progress bar
				// Draw the bar using barImage between left and right buttons, the bar is from left to right
				for(int bx = element->x + GUI_BAR_X_OFFSET + barLeft->w;
					bx < element->x + GUI_BAR_X_OFFSET + barLeft->w + GUI_BAR_WIDTH;
					bx += barImage->w) {
					render_queue_submit_sprite(renderQueue, z, barImage, bx, element->y + GUI_BAR_Y_OFFSET, RND_FLAG_NORMAL);
				}

				// Draw barBall in position based on value
				uint8_t value = element->typed.bar.getValue(context);
				uint8_t maxValue = element->typed.bar.getMaxValue(context);
				int ballX = element->x + GUI_BAR_X_OFFSET + barLeft->w;
				ballX += (value * (GUI_BAR_LANE_WIDTH)) / maxValue;
				render_queue_submit_sprite(renderQueue, z + 1, barBall, ballX, element->y + GUI_BAR_Y_OFFSET, RND_FLAG_NORMAL);
				break;
			}
			case GUI_ELEMENT_VERTICAL_BAR: {
				BITMAP *barUpOff = game_gfx_get_icon(GAME_ICON_BAR_UP_OFF);
				BITMAP *barUpOn = game_gfx_get_icon(GAME_ICON_BAR_UP_ON);
				BITMAP *barDownOff = game_gfx_get_icon(GAME_ICON_BAR_DOWN_OFF);
				BITMAP *barDownOn = game_gfx_get_icon(GAME_ICON_BAR_DOWN_ON);
				BITMAP *barImage = game_gfx_get_icon(GAME_ICON_BAR_VERTICAL);
				BITMAP *barBall = game_gfx_get_icon(GAME_ICON_OPTION_ON);

				BITMAP *barUp = barUpOff;
				BITMAP *barDown = barDownOff;

				if (context->mouseStatus.isLeftDown) {
					GuiVerticalBarPosition vBarPosition = game_gui_mouse_in_vertical_bar(context, element);
					switch (vBarPosition) {
						case GUI_VERTICAL_BAR_UP_BUTTON:
							barUp = barUpOn;
							break;
						case GUI_VERTICAL_BAR_DOWN_BUTTON:
							barDown = barDownOn;
							break;
						case GUI_VERTICAL_BAR_CENTER:
						case GUI_VERTICAL_BAR_NONE:
						default:
							break;
					}
				}

				render_queue_submit_sprite(renderQueue, z, barUp,
										   element->x + GUI_VERTICAL_BAR_X_OFFSET, element->y + GUI_VERTICAL_BAR_Y_OFFSET, RND_FLAG_NORMAL);
				render_queue_submit_sprite(renderQueue, z, barDown,
										   element->x + GUI_VERTICAL_BAR_X_OFFSET, element->y + GUI_VERTICAL_BAR_Y_OFFSET + GUI_VERTICAL_BAR_HEIGHT + barUp->h, RND_FLAG_NORMAL);

				// Render vertical progress bar
				// Draw the bar using barImage between up and down buttons, the bar is from top to bottom
				for (int by = element->y + GUI_VERTICAL_BAR_Y_OFFSET + barUp->h;
					 by < element->y + GUI_VERTICAL_BAR_Y_OFFSET + barUp->h + GUI_VERTICAL_BAR_HEIGHT;
					 by += barImage->h) {
					render_queue_submit_sprite(renderQueue, z, barImage, element->x + GUI_VERTICAL_BAR_X_OFFSET, by, RND_FLAG_NORMAL);
				}

				// Draw barBall in position based on value
				uint8_t value = element->typed.vBar.getValue(context);
				uint8_t maxValue = element->typed.vBar.getMaxValue(context);
				int ballY = element->y + GUI_VERTICAL_BAR_Y_OFFSET + barUp->h;
				ballY += (value * (GUI_VERTICAL_BAR_LANE_HEIGHT)) / max_val(1, maxValue);
				render_queue_submit_sprite(renderQueue, z + 1, barBall, element->x + GUI_VERTICAL_BAR_X_OFFSET, ballY, RND_FLAG_NORMAL);
				break;
			}
			case GUI_ELEMENT_RECTANGLE: {
				GuiRectangle* rectangle = &element->typed.rectangle;
				render_queue_submit_rect(renderQueue, z, element->x, element->y,
										 element->x + rectangle->size.width - 1,
										 element->y + rectangle->size.height - 1,
										 rectangle->color);
				break;
			}
			case GUI_ELEMENT_FILL_RECTANGLE: {
				GuiFillRectangle* fillRectangle = &element->typed.fillRectangle;
				render_queue_submit_rect_fill(renderQueue, z, element->x, element->y,
											  element->x + fillRectangle->size.width - 1,
											  element->y + fillRectangle->size.height - 1,
											  fillRectangle->color);
				break;
			}
			case GUI_ELEMENT_SPRITE: {
				render_queue_submit_sprite(renderQueue, z, *element->typed.image.bitmap, element->x, element->y, RND_FLAG_NORMAL);
				break;
			}
		}
	}
}

#ifndef GUI_H
#define GUI_H
#include "game/game_lib.h"

#define MENU_BACK_X 80
#define MENU_BACK_Y 25
#define MENU_BACK_WIDTH 160
#define MENU_BACK_HEIGHT 160
#define MENU_TITLE_Y_OFFSET 10

#define BUTTON_HEIGHT 18

typedef uint8_t (*GuiGetValueFunc)(const GameContext *context);
typedef uint8_t (*GuiGetMaxValueFunc)(const GameContext *context);
typedef uint8_t (*GuiGetMinValueFunc)(const GameContext *context);
typedef void (*GuiSetValueFunc)(GameContext *context, uint8_t value);
typedef void (*GuiButtonActionFunc)(GameContext *context);
typedef char * (*GuiGetStringFunc)(const GameContext *context);
typedef char * (*GuiGetStringIndexedFunc)(const GameContext *context, uint8_t index);
typedef BITMAP * (*GuiGetIconIndexedFunc)(const GameContext *context, uint8_t index);

typedef enum {
    GUI_ELEMENT_IMAGE,
    GUI_ELEMENT_BUTTON,
    GUI_ELEMENT_CHECK,
    GUI_ELEMENT_BAR,
    GUI_ELEMENT_VERTICAL_BAR,
    GUI_ELEMENT_OPTION,
    GUI_ELEMENT_TEXT,
    GUI_ELEMENT_CUSTOM_TEXT,
    GUI_ELEMENT_CUSTOM_TEXT_ROWS,
    GUI_ELEMENT_RECTANGLE,
    GUI_ELEMENT_FILL_RECTANGLE,
} GuiElementType;

typedef struct {
    GuiButtonActionFunc action;
    Size size;
    uint8_t fit;
} GuiButton;

typedef struct {
    BITMAP** bitmap;
} GuiImage;

typedef struct {
    GuiGetValueFunc getValue;
    GuiSetValueFunc setValue;
} GuiCheck;

typedef struct {
    GameTextIdEnum textId;
    int textColor, textBackground, shadowTextColor;
    char hotkey;
    uint8_t value;
} GuiOptionValue;

typedef struct {
    GuiGetValueFunc getValue;
    GuiSetValueFunc setValue;
    GuiOptionValue* optionValues;
    uint8_t optionValuesNumber;
} GuiOption;

typedef struct {
    // If maxX is > 0, the text will be centered in the area defined by (x, maxX)
    int maxX;
    int maxWidth, maxHeight;
} GuiText;

typedef struct {
    // If maxX is > 0, the text will be centered in the area defined by (x, maxX)
    int maxX;
    int maxWidth, maxHeight;
    GuiGetStringFunc text;
} GuiCustomText;

typedef struct {
    // If maxX is > 0, the text will be centered in the area defined by (x, maxX)
    int maxX;
    int maxWidth, maxHeight;
    int ySeparation, numRows;
    int selectedTextColor;
    GuiGetStringIndexedFunc getText;
    GuiGetValueFunc getOffsetValue;
    GuiGetValueFunc getSelectedValue;
    GuiGetValueFunc getMaxRow;
} GuiCustomTextRows;

typedef struct {
    GuiGetValueFunc getValue;
    GuiSetValueFunc setValue;
    GuiGetMinValueFunc getMinValue;
    GuiGetMaxValueFunc getMaxValue;
    uint8_t valueInc;
} GuiBar;

typedef struct {
    GuiGetValueFunc getValue;
    GuiSetValueFunc setValue;
    GuiGetMinValueFunc getMinValue;
    GuiGetMaxValueFunc getMaxValue;
    uint8_t valueInc;
} GuiVerticalBar;

typedef struct {
    Size size;
    int color;
} GuiRectangle;

typedef struct {
    Size size;
    int color;
} GuiFillRectangle;

typedef struct {
    int x, y, z;
    GuiElementType type;
    GameTextIdEnum textId;
    int textColor, textBackground, shadowTextColor;
    char hotkey;
    union {
        GuiImage image;
        GuiButton button;
        GuiCheck check;
        GuiBar bar;
        GuiVerticalBar vBar;
        GuiOption option;
        GuiText text;
        GuiCustomText customText;
        GuiCustomTextRows customTextRows;
        GuiRectangle rectangle;
        GuiFillRectangle fillRectangle;
    } typed;
} GuiElement;

typedef struct {
    GuiElement* elements;
    uint8_t elementsCount;
} GuiScreen;

void game_gui_handle(GameContext *context, GuiScreen* guiScreen);
void game_gui_render_queue_submit(GameContext *context, RenderQueue* renderQueue, GuiScreen* guiScreen);

#endif /* GUI_H */

#ifndef GUI_H
#define GUI_H
#include "../game_lib.h"

typedef uint8_t (*GuiGetValueFunc)(const GameContext *context);
typedef uint8_t (*GuiGetMaxValueFunc)(const GameContext *context);
typedef uint8_t (*GuiGetMinValueFunc)(const GameContext *context);
typedef void (*GuiSetValueFunc)(GameContext *context, uint8_t value);
typedef void (*GuiButtonActionFunc)(GameContext *context);

typedef enum {
    GUI_ELEMENT_IMAGE,
    GUI_ELEMENT_BUTTON,
    GUI_ELEMENT_CHECK,
    GUI_ELEMENT_BAR,
    GUI_ELEMENT_OPTION,
    GUI_ELEMENT_TEXT,
} GuiElementType;

typedef struct {
    GuiButtonActionFunc action;
    Size size;
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
    int textColor, textBackground;
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
} GuiText;

typedef struct {
    GuiGetValueFunc getValue;
    GuiSetValueFunc setValue;
    GuiGetMinValueFunc getMinValue;
    GuiGetMaxValueFunc getMaxValue;
    uint8_t valueInc;
} GuiBar;

typedef struct {
    int x, y, z;
    GuiElementType type;
    GameTextIdEnum textId;
    int textColor, textBackground;
    char hotkey;
    union {
        GuiImage image;
        GuiButton button;
        GuiCheck check;
        GuiBar bar;
        GuiOption option;
        GuiText text;
    } typed;
} GuiElement;

typedef struct {
    GuiElement* elements;
    uint8_t elementsCount;
} GuiScreen;

void game_gui_handle(GameContext *context, GuiScreen* guiScreen);
void game_gui_render_queue_submit(GameContext *context, RenderQueue* renderQueue, GuiScreen* guiScreen);

#endif /* GUI_H */

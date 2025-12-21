#ifndef GAME_TEXT_H
#define GAME_TEXT_H
#include "../game_lib.h"

typedef enum {
    GAME_TEXT_ID_SELECTED_UNITS = 0,
    GAME_TEXT_ID_NUMBER_OF_TEXTS
} GameTextIdEnum;

typedef enum {
    LANGUAGE_SPANISH = 0,
    LANGUAGE_ENGLISH = 1,
} GameLanguageEnum;

InitializationStatusEnum game_text_init_system(GameLanguageEnum language);
InitializationStatusEnum game_text_set_language(GameLanguageEnum language);

#endif /* GAME_TEXT_H */

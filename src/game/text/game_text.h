#ifndef GAME_TEXT_H
#define GAME_TEXT_H
#include "../game_lib.h"

typedef enum {
    GAME_TEXT_ID_WELCOME = 0,
    GAME_TEXT_ID_SELECT_UNIT,
    GAME_TEXT_ID_MOVE_UNIT,
    GAME_TEXT_ID_ATTACK_UNIT,
    GAME_TEXT_ID_UNIT_DIED,
    GAME_TEXT_ID_GAME_OVER,
    GAME_TEXT_ID_VICTORY,
    GAME_TEXT_ID_DEFEAT,
    GAME_TEXT_ID_NUMBER_OF_TEXTS
} GameTextIdEnum;

typedef enum {
    LANGUAGE_SPANISH = 0,
    LANGUAGE_ENGLISH = 1,
} GameLanguageEnum;

InitializationStatusEnum game_text_init_system(GameLanguageEnum language);
InitializationStatusEnum game_text_set_language(GameLanguageEnum language);

#endif /* GAME_TEXT_H */

#ifndef GAME_TEXT_H
#define GAME_TEXT_H
#include "../game_lib.h"

#define DEFAULT_LANGUAGE LANGUAGE_SPANISH

InitializationStatusEnum game_text_init_system(GameLanguageEnum language);
InitializationStatusEnum game_text_set_language(GameLanguageEnum language);

#endif /* GAME_TEXT_H */

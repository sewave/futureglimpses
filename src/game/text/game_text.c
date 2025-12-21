#include "game_text.h"

#define GAME_TEXT_FILE_ES "assets/txt/es.txt"
#define GAME_TEXT_FILE_EN "assets/txt/en.txt"

InitializationStatusEnum game_text_init_system(GameLanguageEnum language) {
    printf("Loading text system...");
    return text_init_system(GAME_TEXT_ID_NUMBER_OF_TEXTS) == INITIALIZATION_OK &&
           game_text_set_language(language) == INITIALIZATION_OK
           ? INITIALIZATION_OK : INITIALIZATION_ERROR;
}

InitializationStatusEnum game_text_set_language(GameLanguageEnum language) {
    const char * filename;
    switch (language) {
        case LANGUAGE_SPANISH:
            filename = GAME_TEXT_FILE_ES;
            break;
        case LANGUAGE_ENGLISH:
            filename = GAME_TEXT_FILE_EN;
            break;
        default:
            filename = GAME_TEXT_FILE_EN;
            break;
    }
    return text_load_texts_from_file(filename);
}

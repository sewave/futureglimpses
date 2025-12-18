#include "text.h"

static char** texts = NULL;
static uint16_t textsCount = 0;

static void text_free_texts() {
    if (texts != NULL) {
        for (uint16_t i = 0; i < textsCount; i++) {
            if (texts[i] != NULL) {
                free(texts[i]);
                texts[i] = NULL;
            }
        }
        textsCount = 0;
    }
}

InitializationStatusEnum text_init_system(uint16_t numberOfTexts) {
    texts = (char**) malloc(sizeof(char*) * numberOfTexts);
    if (texts == NULL) return INITIALIZATION_ERROR;
    textsCount = numberOfTexts;
    for (uint16_t i = 0; i < textsCount; i++) texts[i] = NULL;
    return INITIALIZATION_OK;
}

void text_free_all() {
    text_free_texts();
    if (texts != NULL) {
        free(texts);
        texts = NULL;
        textsCount = 0;
    }
}

const char* text_get_by_id(uint16_t textId) {
    if (texts == NULL || textId >= textsCount) return NULL;
    return texts[textId];
}

InitializationStatusEnum text_load_texts_from_file(const char * filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return INITIALIZATION_ERROR;
    text_free_texts();

    char buffer[2048];
    uint16_t currentTextId = 0;

    while (fgets(buffer, sizeof(buffer), file) != NULL && currentTextId < textsCount) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        texts[currentTextId] = (char*) malloc(len + 1);
        if (texts[currentTextId] != NULL) {
            strcpy(texts[currentTextId], buffer);
        }
        currentTextId++;
    }

    fclose(file);
    return INITIALIZATION_OK;
}

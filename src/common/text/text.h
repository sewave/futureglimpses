#ifndef TEXT_H
#define TEXT_H
#include "../common_lib.h"

InitializationStatusEnum text_init_system(uint16_t numberOfTexts);
void text_free_all();
InitializationStatusEnum text_load_texts_from_file(const char * filename);
const char* text_get_by_id(uint16_t textId);

#endif /* TEXT_H */

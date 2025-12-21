#include "text.h"

#define CONTROL_SEQUENCE "^"
#define COLOR_CODE_LENGTH 3
#define CONTROL_LENGTH (strlen(CONTROL_SEQUENCE) + COLOR_CODE_LENGTH)

static char **texts = NULL;
static uint16_t totalTexts = 0;

static void text_free_texts() {
	if (texts != NULL) {
		for (uint16_t i = 0; i < totalTexts; i++) {
			if (texts[i] != NULL) {
				free(texts[i]);
				texts[i] = NULL;
			}
		}
	}
}

InitializationStatusEnum text_init_system(uint16_t numberOfTexts) {
	texts = (char **) malloc(sizeof(char *) * numberOfTexts);
	if (texts == NULL) return INITIALIZATION_ERROR;
	totalTexts = numberOfTexts;
	for (uint16_t i = 0; i < totalTexts; i++) texts[i] = NULL;
	return INITIALIZATION_OK;
}

void text_free_all() {
	text_free_texts();
	if (texts != NULL) {
		free(texts);
		texts = NULL;
		totalTexts = 0;
	}
}

const char *text_get_by_id(uint16_t textId) {
	if (texts == NULL || textId >= totalTexts) return NULL;
	return texts[textId];
}

InitializationStatusEnum text_load_texts_from_file(const char *filename) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) return INITIALIZATION_ERROR;
	text_free_texts();

	char buffer[2048];
	uint16_t currentTextId = 0;

	while (fgets(buffer, sizeof(buffer), file) != NULL && currentTextId < totalTexts) {
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0';
		}
		texts[currentTextId] = (char *) malloc(len + 1);
		if (texts[currentTextId] != NULL) {
			strcpy(texts[currentTextId], buffer);
		}
		currentTextId++;
	}

	fclose(file);
	return INITIALIZATION_OK;
}

void text_out_multicolor(struct BITMAP *bmp, const struct FONT *f, const char *str, int x, int y, int color, int bg) {
	const char *currentPtr = str;
	const char *nextMarker = NULL;
	int currentX = x;
	int currentColor = color;
	char segmentBuffer[1024];
	int segmentLength;

	while (*currentPtr != '\0') {
		nextMarker = strstr(currentPtr, CONTROL_SEQUENCE);
		if (nextMarker == NULL) {
			segmentLength = strlen(currentPtr);
		} else {
			segmentLength = nextMarker - currentPtr;
			if (strlen(nextMarker) < CONTROL_LENGTH) {
				nextMarker = NULL;
				segmentLength = strlen(currentPtr);
			}
		}
		if (segmentLength > 0) {
			strncpy(segmentBuffer, currentPtr, segmentLength);
			segmentBuffer[segmentLength] = '\0';
			textout_ex(bmp, f, segmentBuffer, currentX, y, currentColor, bg);
			currentX += text_length(f, segmentBuffer);
		}
		if (nextMarker != NULL) {
			unsigned int colorIndex = 0;
			if (sscanf(nextMarker + strlen(CONTROL_SEQUENCE), "%3x", &colorIndex) == 1) {
				currentColor = colorIndex;
			}
			currentPtr = nextMarker + CONTROL_LENGTH;
		} else {
			currentPtr += segmentLength;
		}
	}
}

#include "common/text.h"
#include "common/common_lib.h"
#include <stdio.h>

#define CONTROL_SEQUENCE "^"
#define COLOR_CODE_LENGTH 3
#define CONTROL_LENGTH (strlen(CONTROL_SEQUENCE) + COLOR_CODE_LENGTH)
#define TEXTOUT_EX_BOX_BUFFER 2048
#define WORD_DELIMITER " "

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
	text_out_multicolor_shadow(bmp, f, str, x, y, color, bg, TRANSPARENT_COLOR);
}

void text_out_multicolor_shadow(struct BITMAP *bmp, const struct FONT *f, const char *str, int x, int y, int color, int bg, int shadowColor) {
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
			if(shadowColor != TRANSPARENT_COLOR) textout_ex(bmp, f, segmentBuffer, currentX + 1, y + 1, shadowColor, bg);
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

void text_out_box(BITMAP *bmp, FONT *font, const char *str, int x, int y, int maxWidth, int maxHeight, int color, int bg) {
	text_out_box_shadow(bmp, font, str, x, y, maxWidth, maxHeight, color, bg, TRANSPARENT_COLOR);
}

void text_out_box_shadow(BITMAP *bmp, FONT *font, const char *str, int x, int y, int maxWidth, int maxHeight, int color, int bg, int shadowColor) {
	char tempBuffer[TEXTOUT_EX_BOX_BUFFER]; // Temporary buffer to hold the string
    char *wordPtr;
    char currentLine[TEXTOUT_EX_BOX_BUFFER] = "";
    int currentY = y;
    int fontHeight = text_height(font);

    // Copy original text to avoid modifying the constant input
    strncpy(tempBuffer, str, sizeof(tempBuffer) - 1);
    tempBuffer[sizeof(tempBuffer) - 1] = '\0';

    // Tokenize text by spaces to process word by word
    wordPtr = strtok(tempBuffer, WORD_DELIMITER);

	char testLine[TEXTOUT_EX_BOX_BUFFER];
    while (wordPtr != NULL) {
        // Prepare a test line: current line + the next word
        if (strlen(currentLine) == 0) {
            strcpy(testLine, wordPtr);
        } else {
            sprintf(testLine, "%s %s", currentLine, wordPtr);
        }

        // Check if the test line fits in the box width
        if (text_length(font, testLine) <= maxWidth) {
            // It fits, update the current line
            strcpy(currentLine, testLine);
        } else {
            // It doesn't fit, print the current line and start a new one
			if(shadowColor != TRANSPARENT_COLOR) {
				textout_ex(bmp, font, currentLine, x + 1, currentY + 1, shadowColor, bg);
			}
            textout_ex(bmp, font, currentLine, x, currentY, color, bg);
            
            currentY += fontHeight + 2; // Move to next line (2 pixels of padding)
            strcpy(currentLine, wordPtr);

            // Safety check: Stop if we exceed the box height
            if (currentY + fontHeight > y + maxHeight) return;
        }

        wordPtr = strtok(NULL, WORD_DELIMITER);
    }

    // Print the last remaining line if there is space
    if (strlen(currentLine) > 0 && (currentY + fontHeight <= y + maxHeight)) {
		if(shadowColor != TRANSPARENT_COLOR) {
			textout_ex(bmp, font, currentLine, x + 1, currentY + 1, shadowColor, bg);
		}
        textout_ex(bmp, font, currentLine, x, currentY, color, bg);
    }
}

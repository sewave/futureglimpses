#include <stdio.h>
#include <allegro/text.h>
#include "common/text.h"
#include "common/video.h"

#define CONTROL_SEQUENCE "^"
#define COLOR_CODE_LENGTH 3
#define CONTROL_LENGTH (strlen(CONTROL_SEQUENCE) + COLOR_CODE_LENGTH)
#define TEXTOUT_EX_BOX_BUFFER 2048
#define WORD_DELIMITER " "
const static char* UNKNOWN_TEXT = "???";
static char buffer[TEXTOUT_EX_BOX_BUFFER];
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
	if (texts == NULL || textId >= totalTexts) return UNKNOWN_TEXT;
	return texts[textId];
}

InitializationStatusEnum text_load_texts_from_file(const char *filename) {
	FILE *file = fopen(filename, "r");
	if (file == NULL) return INITIALIZATION_ERROR;
	text_free_texts();

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
	const char *ptr = str;
	int currentX = x;
	int currentColor = color;
	int bufferLen;
	const int controlLen = strlen(CONTROL_SEQUENCE);

	while (*ptr != '\0') {
		// Find next control sequence
		const char *nextCtrl = strstr(ptr, CONTROL_SEQUENCE);

		// Calculate segment length up to control sequence or end of string
		if (nextCtrl == NULL) {
			bufferLen = strlen(ptr);
			nextCtrl = ptr + bufferLen;// Point to end for later
		} else {
			bufferLen = nextCtrl - ptr;
		}

		// Copy and render segment if there's text
		if (bufferLen > 0) {
			if (bufferLen >= sizeof(buffer)) bufferLen = sizeof(buffer) - 1;
			strncpy(buffer, ptr, bufferLen);
			buffer[bufferLen] = '\0';

			if (shadowColor != TRANSPARENT_COLOR) {
				textout_ex(bmp, f, buffer, currentX + 1, y + 1, shadowColor, bg);
			}
			textout_ex(bmp, f, buffer, currentX, y, currentColor, bg);
			currentX += text_length(f, buffer);
		}

		ptr += bufferLen;

		// Process control sequence if found
		if (*ptr != '\0' && strncmp(ptr, CONTROL_SEQUENCE, controlLen) == 0) {
			// Check if we have enough characters for a valid control sequence (^ + 3 hex)
			if (strlen(ptr) >= CONTROL_LENGTH) {
				unsigned int colorIndex = 0;
				// Try to parse the hex color code
				if (sscanf(ptr + controlLen, "%3x", &colorIndex) == 1) {
					currentColor = colorIndex;
					ptr += CONTROL_LENGTH;
				} else {
					// Invalid format, treat ^ as regular character
					ptr++;
				}
			} else {
				// Not enough characters, stop parsing
				break;
			}
		}
	}
}

void text_out_box(BITMAP *bmp, FONT *font, const char *str, int x, int y, int maxWidth, int maxHeight, int color, int bg) {
	text_out_box_shadow(bmp, font, str, x, y, maxWidth, maxHeight, color, bg, TRANSPARENT_COLOR);
}

void text_out_box_shadow(BITMAP *bmp, FONT *font, const char *str, int x, int y, int maxWidth, int maxHeight, int color, int bg, int shadowColor) {
	strncpy(buffer, str, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';// Ensure null-termination

	int currentY = y;
	int fontHeight = text_height(font);
	char *lineStart = buffer;
	char *lastFitPtr = NULL;
	char *testPtr = NULL;
	char *segmentEnd = NULL;
	char savedChar;
	int lineLen;

	while (*lineStart != '\0') {
		lastFitPtr = lineStart;
		segmentEnd = lineStart + strlen(lineStart);// Cache the end of remaining string
		testPtr = strchr(lineStart, ' ');
		if (testPtr == NULL) testPtr = segmentEnd;

		// Try to fit as many words as possible on this line
		while (testPtr <= segmentEnd) {
			// Temporarily null-terminate and test
			savedChar = *testPtr;
			*testPtr = '\0';

			lineLen = text_length(font, lineStart);

			if (lineLen <= maxWidth) {
				// This fits, save the position
				lastFitPtr = testPtr;
				*testPtr = savedChar;

				// If we reached the end, we're done with this line
				if (testPtr == segmentEnd) break;

				// Look for next space
				testPtr = strchr(testPtr + 1, ' ');
				if (testPtr == NULL) testPtr = segmentEnd;
			} else {
				// Doesn't fit, restore and decide what to do
				*testPtr = savedChar;

				// If nothing fits and we're at the start, show at least until the first space
				// (word is too long, but we must show something to avoid infinite loop)
				if (lastFitPtr == lineStart && testPtr != lineStart) {
					lastFitPtr = testPtr;// Show the long word anyway
				}
				break;
			}
		}

		// Render the line that fits (if there's content)
		if (lastFitPtr > lineStart) {
			savedChar = *lastFitPtr;
			*lastFitPtr = '\0';

			if (shadowColor != TRANSPARENT_COLOR) {
				textout_ex(bmp, font, lineStart, x + 1, currentY + 1, shadowColor, bg);
			}
			textout_ex(bmp, font, lineStart, x, currentY, color, bg);

			*lastFitPtr = savedChar;
		}

		currentY += fontHeight + 2;
		if (currentY + fontHeight > y + maxHeight) return;

		// Move to next line: skip the trailing spaces
		lineStart = lastFitPtr;
		while (*lineStart == ' ') lineStart++;
	}
}

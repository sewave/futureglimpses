#include "common/message.h"
#include "common/video.h"

#define MAX_MESSAGES 5
#define MAX_SLOT_INDEX MAX_MESSAGES - 1
#define MAX_TEXT_LENGTH 256
typedef struct {
	char messageText[MAX_TEXT_LENGTH];
	uint16_t timeLeft;
	int color;
	int background;
	int shadowColor;
} Message;

static int messageStartY;
static int messageYInc;
static int messageX;
static int messageZ;
static int nextSlotIndex;
static Message messageLog[MAX_MESSAGES];

void message_init(int pMessageX, int pMessageStartY, int pMessageYInc, int pMessageZ) {
	messageX = pMessageX;
	messageStartY = pMessageStartY;
	messageYInc = pMessageYInc;
	messageZ = pMessageZ;
	message_reset();
}

void message_reset() {
	nextSlotIndex = 0;
}

void message_add_to_queue(const char *message, uint16_t showTime, int color, int background) {
	message_add_to_queue_shadow(message, showTime, color, background, TRANSPARENT_COLOR);
}

void message_add_to_queue_shadow(const char *message, uint16_t showTime, int color, int background, int shadowColor) {
	if (nextSlotIndex >= MAX_MESSAGES) return;
	Message *ptr = &messageLog[nextSlotIndex];
	strncpy(ptr->messageText, message, MAX_TEXT_LENGTH - 1);
	ptr->messageText[MAX_TEXT_LENGTH - 1] = '\0';
	ptr->timeLeft = showTime;
	ptr->color = color;
	ptr->background = background;
	ptr->shadowColor = shadowColor;
	nextSlotIndex++;
}

void message_render_queue_submit(RenderQueue *renderQueue, FONT *font) {
	if(nextSlotIndex) {
		int yPos = messageStartY;
		Message *currentMessage = &messageLog[nextSlotIndex - 1];
		for (int i = 0; i < nextSlotIndex; i++, currentMessage--, yPos += messageYInc) {
			render_queue_submit_text_multicolor_shadow(renderQueue, messageZ, font,
												currentMessage->messageText, messageX, yPos, currentMessage->color,
												currentMessage->background, currentMessage->shadowColor);
		}
	}
}

void message_update() {
	for (int i = 0; i < nextSlotIndex; i++) {
		Message *currentMessage = &messageLog[i];
		if (--currentMessage->timeLeft == 0) {
			for (int j = i; j < nextSlotIndex - 1; j++) {
				messageLog[j] = messageLog[j + 1];
			}
			nextSlotIndex--;
		}
	}
}

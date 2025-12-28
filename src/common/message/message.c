#include "message.h"

#define MAX_MESSAGES 5
#define MAX_TEXT_LENGTH 256
typedef struct {
	char messageText[MAX_TEXT_LENGTH];
	uint16_t timeLeft;
	int color;
	int background;
	uint8_t isActive;
} Message;

static int messageStartY = 0;
static int messageYInc = 0;
static int messageX = 0;
static int messageZ;
static Message messageLog[MAX_MESSAGES];
static const Message *endPtr = messageLog + MAX_MESSAGES;

void message_init(int pMessageX, int pMessageStartY, int pMessageYInc, int pMessageZ) {
	messageX = pMessageX;
	messageStartY = pMessageStartY;
	messageYInc = pMessageYInc;
    messageZ = pMessageZ;
	message_reset();
}

void message_reset() {
	Message *ptr = messageLog;
	while (ptr < endPtr) {
		ptr->isActive = FALSE;
		ptr++;
	}
}

void message_add_to_queue(const char *message, uint16_t showTime, int color, int background) {
	Message *ptr = messageLog;
	while (ptr < endPtr) {
		if (!ptr->isActive) {
			strncpy(ptr->messageText, message, MAX_TEXT_LENGTH - 1);
			ptr->messageText[MAX_TEXT_LENGTH - 1] = '\0';
			ptr->timeLeft = showTime;
			ptr->color = color;
			ptr->background = background;
			ptr->isActive = TRUE;
			return;
		}
		ptr++;
	}
}

void message_render_queue_submit(RenderQueue *renderQueue, FONT* font) {
	int yPos = messageStartY;

	Message *currentMessage = messageLog + MAX_MESSAGES - 1;

	while (currentMessage >= messageLog) {
		if (currentMessage->isActive) {
            render_queue_submit_text_multicolor(renderQueue, messageZ, font,
                currentMessage->messageText, messageX, yPos, currentMessage->color,
                currentMessage->background);
			yPos += messageYInc;
		}
		currentMessage--;
	}
}

void message_update() {
	Message *ptr = messageLog;
	while (ptr < endPtr) {
		if (ptr->isActive && --ptr->timeLeft == 0) ptr->isActive = FALSE;
		ptr++;
	}
}

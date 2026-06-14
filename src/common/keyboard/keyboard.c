#include <allegro/keyboard.h>
#include "common/keyboard.h"

static char previousKey[KEY_MAX];

uint8_t keyboard_is_key_down(int keyCode) {
	return key[keyCode];
}

uint8_t keyboard_is_key_pressed(int keyCode) {
	return key[keyCode] && !previousKey[keyCode];
}

uint8_t keyboard_is_key_released(int keyCode) {
	return !key[keyCode] && previousKey[keyCode];
}

void keyboard_update() {
	memcpy(previousKey, (char *) key, sizeof(previousKey));
	poll_keyboard();
}

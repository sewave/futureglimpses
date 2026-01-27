#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "common/common.h"

void keyboard_update();
uint8_t keyboard_is_key_down(int keyCode);
uint8_t keyboard_is_key_pressed(int keyCode);
uint8_t keyboard_is_key_released(int keyCode);

#endif /* KEYBOARD_H */

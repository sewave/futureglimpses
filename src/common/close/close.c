#include "common/close.h"
#include <allegro.h>

static volatile uint8_t closeButtonPressed = FALSE;

static void close_button_handler() {
	closeButtonPressed = TRUE;
}
END_OF_FUNCTION(close_button_handler)

void close_install_handler() {
	LOCK_VARIABLE(closeButtonPressed);
	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);
}

uint8_t close_is_pressed() {
	return closeButtonPressed;
}
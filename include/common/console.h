#ifndef CONSOLE_H
#define CONSOLE_H

// --- UNIVERSAL COLOR ENUMERATION ---
typedef enum {
    CONSOLE_COLOR_BLACK,
    CONSOLE_COLOR_BLUE,
    CONSOLE_COLOR_GREEN,
    CONSOLE_COLOR_CYAN,
    CONSOLE_COLOR_RED,
    CONSOLE_COLOR_MAGENTA,
    CONSOLE_COLOR_BROWN,
    CONSOLE_COLOR_LIGHT_GRAY,
    CONSOLE_COLOR_DARK_GRAY,
    CONSOLE_COLOR_LIGHT_BLUE,
    CONSOLE_COLOR_LIGHT_GREEN,
    CONSOLE_COLOR_LIGHT_CYAN,
    CONSOLE_COLOR_LIGHT_RED,
    CONSOLE_COLOR_LIGHT_MAGENTA,
    CONSOLE_COLOR_YELLOW,
    CONSOLE_COLOR_WHITE
} ConsoleColorType;

// --- CURSOR TYPE ENUMERATION ---
typedef enum {
    CONSOLE_CURSOR_HIDDEN,
    CONSOLE_CURSOR_NORMAL,
    CONSOLE_CURSOR_BLOCK
} ConsoleCursorType;

// --- BLINK TYPE ENUMERATION ---
typedef enum {
    CONSOLE_BLINK_OFF,
    CONSOLE_BLINK_ON
} ConsoleBlinkType;

// --- PUBLIC FUNCTION DECLARATIONS ---

// Color and style management
void console_set_text_color(ConsoleColorType textColor);
void console_set_background_color(ConsoleColorType bgColor);
void console_set_blink_state(ConsoleBlinkType blinkState);
void console_reset_styles();

// Screen and cursor utilities
void console_clear_screen();
void console_move_cursor(int x, int y);
void console_set_cursor_type(ConsoleCursorType cursorType);

// Formatted printing
void console_printf(const char* textFormat, ...);
void console_print_box(int x, int y, int width, int height);

#endif // CONSOLE_H

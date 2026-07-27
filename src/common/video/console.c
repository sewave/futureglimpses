#include "common/console.h"
#include <stdio.h>
#include <stdarg.h>

// --- PLATFORM DETECTION ---
#if defined(__DJGPP__) || defined(__MSDOS__)
#define PLATFORM_DOS
#include <conio.h>
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WIN
#include <windows.h>
#else
#define PLATFORM_FALLBACK
#endif

// --- PRIVATE TRANSLATION ARRAYS (Static) ---
#ifdef PLATFORM_WIN
static const int winColorMap[] = {
		0,
		FOREGROUND_BLUE,
		FOREGROUND_GREEN,
		FOREGROUND_GREEN | FOREGROUND_BLUE,
		FOREGROUND_RED,
		FOREGROUND_RED | FOREGROUND_BLUE,
		FOREGROUND_RED | FOREGROUND_GREEN,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		FOREGROUND_INTENSITY,
		FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY};
#endif

// --- PRIVATE INTERNAL STATE (Static & camelCase) ---
static ConsoleColorType currentText = CONSOLE_COLOR_LIGHT_GRAY;
static ConsoleColorType currentBack = CONSOLE_COLOR_BLACK;
static ConsoleBlinkType currentBlink = CONSOLE_BLINK_OFF;

// --- PRIVATE HELPERS (Static) ---
#ifdef PLATFORM_WIN
static void win_update_attributes(void) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WORD attributes = winColorMap[currentText] | (winColorMap[currentBack] << 4);
	if (currentBlink == CONSOLE_BLINK_ON) {
		attributes |= BACKGROUND_INTENSITY;
	}
	SetConsoleTextAttribute(hConsole, attributes);
}
#endif

// --- PUBLIC FUNCTION IMPLEMENTATIONS (snake_case) ---

void console_set_text_color(ConsoleColorType textColor) {
	currentText = textColor;
#ifdef PLATFORM_DOS
	textcolor(currentBlink == CONSOLE_BLINK_ON ? (textColor + BLINK) : textColor);
#elif defined(PLATFORM_WIN)
	win_update_attributes();
#endif
}

void console_set_background_color(ConsoleColorType bgColor) {
	currentBack = bgColor;
#ifdef PLATFORM_DOS
	textbackground(bgColor & 0x07);
#elif defined(PLATFORM_WIN)
	win_update_attributes();
#endif
}

void console_set_blink_state(ConsoleBlinkType blinkState) {
	currentBlink = blinkState;
	console_set_text_color(currentText);// Re-apply to inject blink changes
}

void console_set_cursor_type(ConsoleCursorType cursorType) {
#ifdef PLATFORM_DOS
	switch (cursorType) {
		case CONSOLE_CURSOR_HIDDEN:
			_setcursortype(_NOCURSOR);
			break;
		case CONSOLE_CURSOR_NORMAL:
			_setcursortype(_NORMALCURSOR);
			break;
		case CONSOLE_CURSOR_BLOCK:
			_setcursortype(_SOLIDCURSOR);
			break;
	}
#elif defined(PLATFORM_WIN)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	if (hConsole && GetConsoleCursorInfo(hConsole, &cursorInfo)) {
		if (cursorType == CONSOLE_CURSOR_HIDDEN) {
			cursorInfo.bVisible = FALSE;
		} else {
			cursorInfo.bVisible = TRUE;
			cursorInfo.dwSize = (cursorType == CONSOLE_CURSOR_BLOCK) ? 100 : 20;
		}
		SetConsoleCursorInfo(hConsole, &cursorInfo);
	}
#elif defined(PLATFORM_FALLBACK)
	if (cursorType == CONSOLE_CURSOR_HIDDEN) {
		printf("\033[?25l");
	} else {
		printf("\033[?25h");
		printf(cursorType == CONSOLE_CURSOR_BLOCK ? "\033[2 q" : "\033[6 q");
	}
#endif
}

void console_reset_styles(void) {
	currentText = CONSOLE_COLOR_LIGHT_GRAY;
	currentBack = CONSOLE_COLOR_BLACK;
	currentBlink = CONSOLE_BLINK_OFF;
#ifdef PLATFORM_DOS
	textbackground(CONSOLE_COLOR_BLACK);
	textcolor(CONSOLE_COLOR_LIGHT_GRAY);
#elif defined(PLATFORM_WIN)
	win_update_attributes();
#endif
	console_set_cursor_type(CONSOLE_CURSOR_HIDDEN);
}

void console_printf(const char *textFormat, ...) {
	va_list args;
	va_start(args, textFormat);
#ifdef PLATFORM_DOS
	char buffer[128]; 
    vsprintf(buffer, textFormat, args);
    cprintf("%s", buffer);
#else
	vprintf(textFormat, args);
#endif
	va_end(args);
}

void console_clear_screen(void) {
#ifdef PLATFORM_DOS
	clrscr();
#elif defined(PLATFORM_WIN)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD count, cellCount;
	COORD homeCoords = {0, 0};

	if (!hConsole || !GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;

	FillConsoleOutputCharacter(hConsole, (TCHAR) ' ', cellCount, homeCoords, &count);
	FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
	SetConsoleCursorPosition(hConsole, homeCoords);
#elif defined(PLATFORM_FALLBACK)
	printf("\033[H\033[J");
	fflush(stdout);
#endif
}

void console_move_cursor(int x, int y) {
#ifdef PLATFORM_DOS
	gotoxy(x + 1, y + 1);
#elif defined(PLATFORM_WIN)
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coords = {(SHORT) x, (SHORT) y};
	SetConsoleCursorPosition(hConsole, coords);
#elif defined(PLATFORM_FALLBACK)
	printf("\033[%d;%dH", y + 1, x + 1);
#endif
}

void console_print_box(int x, int y, int width, int height) {
    if (width < 2 || height < 2) return;

    // IBM PC Characters (Extended ASCII) CP437
    const char tl = (char)218; // ┌
    const char tr = (char)191; // ┐
    const char bl = (char)192; // └
    const char br = (char)217; // ┘
    const char hz = (char)196; // ─
    const char vt = (char)179; // │

    int remainingHz = width - 2;

    console_move_cursor(x, y);
    console_printf("%c", tl);
    for (int i = 0; i < remainingHz; i++) {
        console_printf("%c", hz);
    }
    console_printf("%c", tr);

    for (int row = 1; row < height - 1; row++) {
        console_move_cursor(x, y + row);
        console_printf("%c", vt);
        
        for (int i = 0; i < remainingHz; i++) {
            console_printf(" ");
        }
        
        console_printf("%c", vt);
    }

    console_move_cursor(x, y + height - 1);
    console_printf("%c", bl);
    for (int i = 0; i < remainingHz; i++) {
        console_printf("%c", hz);
    }
    console_printf("%c", br);
}

ConsoleCoords console_get_cursor_position(void) {
    ConsoleCoords coords = { 0, 0 };

#ifdef PLATFORM_DOS
    coords.x = wherex() - 1;
    coords.y = wherey() - 1;
#elif defined(PLATFORM_WIN)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    if (hConsole && GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        coords.x = csbi.dwCursorPosition.X;
        coords.y = csbi.dwCursorPosition.Y;
    }
#endif
    return coords;
}


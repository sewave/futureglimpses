#include <stdio.h>
#include <stdlib.h>
#include "game/state/intro.h"
#include <allegro.h>

#define LINE_X 4
#define LINE_Y_SPACING 10
#define LINE_Y_START GAME_INTERNAL_HEIGHT
#define LINE_Y_HALF (GAME_INTERNAL_HEIGHT / 2)
#define NUM_COLORS 17
#define MAX_COLOR (NUM_COLORS - 1)

static uint8_t numLines;
static char ** lines;
static int * linesY;
static int * linesColor;

static void calculate_line_colors() {
	for(int i = 0; i < numLines; i++) {
		int color = MAX_COLOR - ((abs(linesY[i] - LINE_Y_HALF) * MAX_COLOR) / LINE_Y_HALF);
		if(color < 0) color = 0;
		int colorTone = (color * 255) / MAX_COLOR;
		linesColor[i] = makecol8(colorTone, colorTone, colorTone);
	}
}

static void free_lines() {
	for(int i = 0; i < numLines; i++) free(lines[i]);
	free(lines);
	lines = NULL;
	free(linesY);
	linesY = NULL;
	free(linesColor);
	linesColor = NULL;
}

void game_state_intro_init(GameContext *context) {
	// TODO load lines from file
	numLines = 1;
	lines = (char **) calloc(numLines, sizeof(char *));
	lines[0] = strdup("Lorem ipsum dolor sit amet, consectetur");
	linesY = (int *) calloc(numLines, sizeof(int *));
	linesColor = (int *) calloc(numLines, sizeof(int *));

	int lineY = LINE_Y_START + LINE_Y_SPACING;
	for(int i = 0; i < numLines; i++) {
		linesY[i] = lineY;
		lineY += LINE_Y_SPACING;
	}
	video_fade_in_init(DEFAULT_FADE_SPEED, context->mainPalette);
}

GameStateEnum game_state_intro_update(GameContext *context) {
	for(int i = 0; i < numLines; i++) linesY[i]--;
	calculate_line_colors();
	if(linesY[numLines - 1] < -LINE_Y_SPACING || keyboard_is_key_pressed(KEY_ESC)) {
		free_lines();
		video_fade_out_init(DEFAULT_FADE_SPEED);
		return GAME_STATE_TITLE;
	}
	return GAME_STATE_INTRO;
}

void game_state_intro_render(GameContext *context, RenderQueue *renderQueue) {
	render_queue_submit_rect_fill(renderQueue, 0, 0, 0, GAME_INTERNAL_WIDTH, GAME_INTERNAL_HEIGHT, PAL_COLOR_BLACK);
	for(int i = 0; i < numLines; i++) {
		int lineY = linesY[i];
		if(lineY > -LINE_Y_SPACING && lineY < LINE_Y_START) {
			render_queue_submit_text(
				renderQueue, 1, context->gameFont, lines[i], LINE_X, lineY,
				linesColor[i], TRANSPARENT_INDEX
			);
		}
	}
}

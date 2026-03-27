#include "common/starfield.h"
#include "common/video.h"
#include <stdio.h>

#define MAX_Z 1024
#define STAR_RESOLUTION 8
#define NUM_COLORS 17
#define MAX_COLOR (NUM_COLORS - 1)
static int width, height, numStars, speed;
static Star *field;
static int* colors;

static void starfield_init_star(Star *s) {
	s->x = (rand() % (width * 2)) - width;
	s->y = (rand() % (height * 2)) - height;
	s->z = (rand() % MAX_Z) + 1;
}

static void starfield_init_colors(PALETTE palette) {
    for(int i = 0; i < NUM_COLORS; i++) {
        int colorTone = (i * 63) / NUM_COLORS;
        colors[i] = video_color_get_best_match(colorTone, colorTone, colorTone, palette);
    }
}

void starfield_init_stars(int pWidth, int pHeight, int pNumStars, int pSpeed, PALETTE palette) {
    width = pWidth;
    height = pHeight;
    numStars = pNumStars;
    speed = pSpeed;

	field = (Star *) calloc(numStars, sizeof(Star));
	if(field == NULL) {
		fprintf(stderr, "Error initializing stars.\n");
		exit(PROGRAM_ERROR);	
	}
    colors = (int*) calloc(NUM_COLORS, sizeof(int*));
    starfield_init_colors(palette);
	for (int i = 0; i < numStars; i++) starfield_init_star(&field[i]);
}

void starfield_update_stars() {
	for (int i = 0; i < numStars; i++) {
		field[i].z -= speed;
		if (field[i].z <= 0) starfield_init_star(&field[i]);
	}
}

void starfield_draw_stars(RenderQueue *renderQueue) {
	for (int i = 0; i < numStars; i++) {
		Star *s = &field[i];
		int sx = ((s->x << STAR_RESOLUTION) / s->z) + (width / 2);
		int sy = ((s->y << STAR_RESOLUTION) / s->z) + (height / 2);
		int color = colors[MAX_COLOR - (s->z * MAX_COLOR) / MAX_Z];
		int size = 0;
		if(s->z <  (2 * MAX_Z) / 3) size = 1;
		if(s->z <  (1 * MAX_Z) / 3) size = 2;
		render_queue_submit_rect_fill(renderQueue, 1, sx, sy, sx + size, sy + size, color);
	}
}

void starfield_free_stars() {
	free(field);
	field = NULL;
    free(colors);
    colors = NULL;
}

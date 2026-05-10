#ifndef STARFIELD_H
#define STARFIELD_H
#include "common/common.h"
#include "common/render_queue.h"
#include <allegro.h>

typedef struct {
	int x, y, z;
} Star;

#define MAX_STAR_Z 1024

void starfield_init_stars(int pWidth, int pHeight, int pNumStars, int pSpeed, PALETTE palette) ;
void starfield_update_stars();
void starfield_draw_stars(RenderQueue *renderQueue);
void starfield_free_stars();

#endif /* STARFIELD_H */

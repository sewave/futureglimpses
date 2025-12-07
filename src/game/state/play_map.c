#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define MINIMAP_X_POS 3
#define MINIMAP_Y_POS 6

#define MOUSE_X_GO_LEFT TILE_SIZE / 2
#define MOUSE_X_GO_RIGHT (GAME_INTERNAL_WIDTH - TILE_SIZE / 2)
#define MOUSE_Y_GO_UP TILE_SIZE / 2
#define MOUSE_Y_GO_DOWN (GAME_INTERNAL_HEIGHT - TILE_SIZE / 2)

static char fpsText[16];
int moveViewportCounter = 0;
GameStateEnum handle_play_map(GameContext *context, RenderQueue *renderQueue) {
	/*if (key[KEY_A] & !keyPrevious[KEY_A]) game_snd_play_sound(GAME_SOUND_SEA_WAVES);
	if (key[KEY_S] & !keyPrevious[KEY_S]) game_snd_play_sound(GAME_SOUND_CLICK);*/

	// TODO change cursor style based on position UP-LEFT, UP-RIGHT, DOWN-LEFT, DOWN-RIGHT, LEFT, RIGHT, UP, DOWN
	int mouseX = mouse_get_x();
	int mouseY = mouse_get_y();

	// If we click the mouse on the minimap, we should move the camera there
	if (mouse_b & 1) {
		if (mouseX >= MINIMAP_X_POS &&
			mouseX <= MINIMAP_X_POS + BOARD_WIDTH &&
			mouseY >= MINIMAP_Y_POS &&
			mouseY <= MINIMAP_Y_POS + BOARD_HEIGHT) {
			context->xPosition = (mouseX - MINIMAP_X_POS - 8) * TILE_SIZE;
			context->yPosition = (mouseY - MINIMAP_Y_POS - 6) * TILE_SIZE;
			if (context->yPosition < 0) context->yPosition = 0;
			if (context->xPosition < 0) context->xPosition = 0;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
	}

	if ((key[KEY_UP] || key[KEY_DOWN] || key[KEY_LEFT] || key[KEY_RIGHT] ||
		mouseX < MOUSE_X_GO_LEFT || mouseX > MOUSE_X_GO_RIGHT ||
		mouseY < MOUSE_Y_GO_UP || mouseY > MOUSE_Y_GO_DOWN) && !(mouse_b & 1)) {
		moveViewportCounter++;
	} else {
		moveViewportCounter = 0;
	}
	int cameraSpeed = 2;

	if (moveViewportCounter >= 1) {
		if (key[KEY_UP] || mouseY < MOUSE_Y_GO_UP) {
			context->yPosition -= cameraSpeed;
			if (context->yPosition < 0) context->yPosition = 0;
		}
		if (key[KEY_DOWN] || mouseY > MOUSE_Y_GO_DOWN) {
			context->yPosition += cameraSpeed;
			if (context->yPosition > MAX_CAMERA_Y_POSITION) context->yPosition = MAX_CAMERA_Y_POSITION;
		}
		if (key[KEY_LEFT] || mouseX < MOUSE_X_GO_LEFT) {
			context->xPosition -= cameraSpeed;
			if (context->xPosition < 0) context->xPosition = 0;
		}
		if (key[KEY_RIGHT] || mouseX > MOUSE_X_GO_RIGHT) {
			context->xPosition += cameraSpeed;
			if (context->xPosition > MAX_CAMERA_X_POSITION) context->xPosition = MAX_CAMERA_X_POSITION;
		}
		moveViewportCounter = 0;
	}

	// Advance units animation
	// TODO only active units
	 GameUnit* unit = context->units;
    for(int i = 0; i < MAX_GAME_UNITS; i++, unit++) {
        if(unit->isActive) {
			game_animation_unit_advance(context, unit);
			game_unit_ai_invoke(context, unit);
		}
    }

	// If there are more ticks to draw, skip queue phase
	if (context->ticksToCatchup) return GAME_STATE_PLAY_MAP;

	render_queue_units(context, renderQueue);
	// TODO queue effects, proyectiles, particles, etc

	// Submit to render the viewport from the renderedBoard
	render_queue_submit_masked_partial(renderQueue, UI_Z_ORDER + 10, context->gameBack, 0, 0, 0,0, 320, 200);

	render_queue_submit_solid_partial(renderQueue, BACKGROUND_Z_ORDER, context->renderedBoard,
									  context->xPosition, context->yPosition,
									  VIEWPORT_X_OFFSET, VIEWPORT_Y_OFFSET,
									  VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	// Minimap
	render_queue_submit_solid(renderQueue, UI_Z_ORDER + 11, context->renderedMinimap,
									  MINIMAP_X_POS, MINIMAP_Y_POS);

	// Minimap viewport window
	render_queue_submit_rect(renderQueue,
							 UI_Z_ORDER + 13,
							 context->xPosition / TILE_SIZE + MINIMAP_X_POS,
							 context->yPosition / TILE_SIZE + MINIMAP_Y_POS,
							 context->xPosition / TILE_SIZE + MINIMAP_X_POS + VIEWPORT_WIDTH_TILES - 1,
							 context->yPosition / TILE_SIZE + MINIMAP_Y_POS + VIEWPORT_HEIGHT_TILES - 1,
							 PAL_COLOR_WHITE);

	render_queue_submit_sprite(renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(), mouse_get_x() - mouse_x_focus, mouse_get_y() - mouse_y_focus, RND_FLAG_NORMAL);
	snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
	render_queue_submit_text(renderQueue, UI_Z_ORDER + 100, font, fpsText, 0, 190, PAL_COLOR_WHITE, -1);

	return GAME_STATE_PLAY_MAP;
}

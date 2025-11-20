#include <stdio.h>
#include <stdlib.h>
#include "..\game_lib.h"
#include <allegro.h>

#define VIEWPORT_WIDTH_TILES 15
#define VIEWPORT_HEIGHT_TILES 11
#define VIEWPORT_WIDTH VIEWPORT_WIDTH_TILES *TILE_SIZE
#define VIEWPORT_HEIGHT VIEWPORT_HEIGHT_TILES *TILE_SIZE
#define MAX_CAMERA_X_POSITION (BOARD_WIDTH - VIEWPORT_WIDTH_TILES)
#define MAX_CAMERA_Y_POSITION (BOARD_HEIGHT - VIEWPORT_HEIGHT_TILES)

#define MOUSE_X_GO_LEFT TILE_SIZE
#define MOUSE_X_GO_RIGHT (GAME_INTERNAL_WIDTH - TILE_SIZE)
#define MOUSE_Y_GO_UP TILE_SIZE
#define MOUSE_Y_GO_DOWN (GAME_INTERNAL_HEIGHT - TILE_SIZE)

static char fpsText[16];
int keyHeldCounter = 0;
GameStateEnum handle_play_map(GameState *gameState, RenderQueue *renderQueue) {
	/*if (key[KEY_A] & !keyPrevious[KEY_A]) game_snd_play_sound(GAME_SOUND_SEA_WAVES);
	if (key[KEY_S] & !keyPrevious[KEY_S]) game_snd_play_sound(GAME_SOUND_CLICK);*/
	// Move camera every 10 frames while arrow key is held down

	// TODO change cursor style based on position UP-LEFT, UP-RIGHT, DOWN-LEFT, DOWN-RIGHT, LEFT, RIGHT, UP, DOWN
	int mouseX = mouse_get_x();
	int mouseY = mouse_get_y();

	if (key[KEY_UP] || key[KEY_DOWN] || key[KEY_LEFT] || key[KEY_RIGHT] ||
		mouseX < MOUSE_X_GO_LEFT || mouseX > MOUSE_X_GO_RIGHT ||
		mouseY < MOUSE_Y_GO_UP || mouseY > MOUSE_Y_GO_DOWN) {
		keyHeldCounter++;
	} else {
		keyHeldCounter = 0;
	}

	if (keyHeldCounter >= 10) {
		if (key[KEY_UP] || mouseY < MOUSE_Y_GO_DOWN) {
			gameState->yPosition -= 1;
			if (gameState->yPosition < 0) gameState->yPosition = 0;
		}
		if (key[KEY_DOWN] || mouseY > MOUSE_Y_GO_UP) {
			gameState->yPosition += 1;
			if (gameState->yPosition > MAX_CAMERA_Y_POSITION) gameState->yPosition = MAX_CAMERA_Y_POSITION;
		}
		if (key[KEY_LEFT] || mouseX < MOUSE_X_GO_LEFT) {
			gameState->xPosition -= 1;
			if (gameState->xPosition < 0) gameState->xPosition = 0;
		}
		if (key[KEY_RIGHT] || mouseX > MOUSE_X_GO_RIGHT) {
			gameState->xPosition += 1;
			if (gameState->xPosition > MAX_CAMERA_X_POSITION) gameState->xPosition = MAX_CAMERA_X_POSITION;
		}
		keyHeldCounter = 0;
	}

	// Submit to render the viewport from the renderedBoard
	render_queue_submit_clear(renderQueue, BACKGROUND_Z_ORDER, 0);
	render_queue_submit_solid_partial(renderQueue, BACKGROUND_Z_ORDER + 1, gameState->renderedBoard,
									  gameState->xPosition * TILE_SIZE, gameState->yPosition * TILE_SIZE,
									  68, 12,
									  VIEWPORT_WIDTH, VIEWPORT_HEIGHT,
									  RND_FLAG_NORMAL);

	render_queue_submit_sprite(renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(), mouse_get_x() - mouse_x_focus, mouse_get_y() - mouse_y_focus, RND_FLAG_HV_FLIP);
	snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
	render_queue_submit_text(renderQueue, 5000, font, fpsText, 10, 10, 1, 0);
	return GAME_STATE_PLAY_MAP;
}

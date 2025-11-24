#include <stdio.h>
#include <stdlib.h>
#include "../game_lib.h"
#include <allegro.h>

#define VIEWPORT_WIDTH_TILES 15
#define VIEWPORT_HEIGHT_TILES 11
#define VIEWPORT_WIDTH VIEWPORT_WIDTH_TILES *TILE_SIZE
#define VIEWPORT_HEIGHT VIEWPORT_HEIGHT_TILES *TILE_SIZE
#define MAX_CAMERA_X_POSITION (BOARD_WIDTH - VIEWPORT_WIDTH_TILES)
#define MAX_CAMERA_Y_POSITION (BOARD_HEIGHT - VIEWPORT_HEIGHT_TILES)

#define MINIMAP_X_POS 3
#define MINIMAP_Y_POS 6

#define MOUSE_X_GO_LEFT TILE_SIZE / 2
#define MOUSE_X_GO_RIGHT (GAME_INTERNAL_WIDTH - TILE_SIZE / 2)
#define MOUSE_Y_GO_UP TILE_SIZE / 2
#define MOUSE_Y_GO_DOWN (GAME_INTERNAL_HEIGHT - TILE_SIZE / 2)

static char fpsText[16];
int moveViewportCounter = 0;
GameStateEnum handle_play_map(GameState *gameState, RenderQueue *renderQueue) {
	/*if (key[KEY_A] & !keyPrevious[KEY_A]) game_snd_play_sound(GAME_SOUND_SEA_WAVES);
	if (key[KEY_S] & !keyPrevious[KEY_S]) game_snd_play_sound(GAME_SOUND_CLICK);*/
	// Move camera every 10 frames while arrow key is held down

	// TODO change cursor style based on position UP-LEFT, UP-RIGHT, DOWN-LEFT, DOWN-RIGHT, LEFT, RIGHT, UP, DOWN
	int mouseX = mouse_get_x();
	int mouseY = mouse_get_y();

	// If we click the mouse on the minimap, we should move the camera there
	if (mouse_b & 1) {
		if (mouseX >= MINIMAP_X_POS &&
			mouseX <= MINIMAP_X_POS + BOARD_WIDTH &&
			mouseY >= MINIMAP_Y_POS &&
			mouseY <= MINIMAP_Y_POS + BOARD_HEIGHT) {
			gameState->xPosition = mouseX - MINIMAP_X_POS - 8;
			gameState->yPosition = mouseY - MINIMAP_Y_POS - 6;
			if (gameState->yPosition < 0) gameState->yPosition = 0;
			if (gameState->xPosition < 0) gameState->xPosition = 0;
			if (gameState->xPosition > MAX_CAMERA_X_POSITION) gameState->xPosition = MAX_CAMERA_X_POSITION;
			if (gameState->yPosition > MAX_CAMERA_Y_POSITION) gameState->yPosition = MAX_CAMERA_Y_POSITION;
		}
	}

	if ((key[KEY_UP] || key[KEY_DOWN] || key[KEY_LEFT] || key[KEY_RIGHT] ||
		mouseX < MOUSE_X_GO_LEFT || mouseX > MOUSE_X_GO_RIGHT ||
		mouseY < MOUSE_Y_GO_UP || mouseY > MOUSE_Y_GO_DOWN) && !(mouse_b & 1)) {
		moveViewportCounter++;
	} else {
		moveViewportCounter = 0;
	}

	if (moveViewportCounter >= 5) {
		if (key[KEY_UP] || mouseY < MOUSE_Y_GO_UP) {
			gameState->yPosition -= 1;
			if (gameState->yPosition < 0) gameState->yPosition = 0;
		}
		if (key[KEY_DOWN] || mouseY > MOUSE_Y_GO_DOWN) {
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
		moveViewportCounter = 0;
	}

	// Submit to render the viewport from the renderedBoard
	render_queue_submit_solid(renderQueue, BACKGROUND_Z_ORDER, gameState->gameBack, 0, 0);

	render_queue_submit_solid_partial(renderQueue, BACKGROUND_Z_ORDER + 1, gameState->renderedBoard,
									  gameState->xPosition * TILE_SIZE, gameState->yPosition * TILE_SIZE,
									  72, 12,
									  VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
	// Minimap
	render_queue_submit_solid(renderQueue, BACKGROUND_Z_ORDER + 1, gameState->renderedMinimap,
									  MINIMAP_X_POS, MINIMAP_Y_POS);

	// Minimap viewport window
	render_queue_submit_rect(renderQueue,
							 BACKGROUND_Z_ORDER + 3,
							 gameState->xPosition + MINIMAP_X_POS,
							 gameState->yPosition + MINIMAP_Y_POS,
							 gameState->xPosition + MINIMAP_X_POS + VIEWPORT_WIDTH_TILES - 1,
							 gameState->yPosition + MINIMAP_Y_POS + VIEWPORT_HEIGHT_TILES - 1,
							 makecol8(255, 255, 255));

	render_queue_submit_sprite(renderQueue, MOUSE_Z_ORDER, mouse_get_cursor_sprite(), mouse_get_x() - mouse_x_focus, mouse_get_y() - mouse_y_focus, RND_FLAG_NORMAL);
	snprintf(fpsText, sizeof(fpsText), "FPS: %.1f", fps_get());
	render_queue_submit_text(renderQueue, 5000, font, fpsText, 0, 190, makecol8(255, 255, 255), -1);

	return GAME_STATE_PLAY_MAP;
}

#include "game/unit/player_attacked.h"
#include "game/sound/game_sound.h"
#include "game/video/game_video.h"

#define COOLDOWN_TICKS SEC_TO_FRAMES(10)
#define ATTACK_SHOWN_TICKS SEC_TO_FRAMES(8)
#define BLINK_TICKS SEC_TO_FRAMES(1)

void player_attacked_init(PlayerAttackedData *playerAttackedData) {
    playerAttackedData->showMessage = FALSE;
    playerAttackedData->cooldown = 0;
    playerAttackedData->xPosition = 0;
    playerAttackedData->yPosition = 0;
    playerAttackedData->blinkCounter = 0;
    playerAttackedData->blinkShowing = TRUE;
}

void player_attacked_update(PlayerAttackedData *playerAttackedData) {
    if(playerAttackedData->cooldown > COOLDOWN_TICKS) playerAttackedData->cooldown = 0;
    if(playerAttackedData->cooldown > 0) playerAttackedData->cooldown--;
}

void player_attacked_register_attack(PlayerAttackedData *playerAttackedData, uint16_t xPosition, uint16_t yPosition) {
    // We are in cooldown, ignore the attack
    if(playerAttackedData->cooldown) return;

    playerAttackedData->showMessage = TRUE;
    playerAttackedData->cooldown = COOLDOWN_TICKS;
    playerAttackedData->xPosition = xPosition;
    playerAttackedData->yPosition = yPosition;
    playerAttackedData->blinkCounter = 0;
    playerAttackedData->blinkShowing = TRUE;
    game_snd_play_sound(GAME_SOUND_INCOMING_ATTACK);
}

void player_attacked_render(PlayerAttackedData *playerAttackedData, RenderQueue *renderQueue) {
    if(!playerAttackedData->cooldown) return;
    if(playerAttackedData->showMessage) {
        message_add_to_queue_shadow(text_get_by_id(GAME_TEXT_ID_PLAYER_ATTACKED),
            ATTACK_SHOWN_TICKS, PAL_COLOR_VIOLET, TRANSPARENT_INDEX, PAL_COLOR_BLACK);
        playerAttackedData->showMessage = FALSE;
    }
    if(playerAttackedData->blinkCounter >= BLINK_TICKS) {
        playerAttackedData->blinkCounter = 0;
        playerAttackedData->blinkShowing = !playerAttackedData->blinkShowing;
    } else {
        playerAttackedData->blinkCounter++;
    }
    int circleRadius;
    if(playerAttackedData->blinkShowing) circleRadius = 3; else circleRadius = 1;
    render_queue_submit_circle(renderQueue, UI_Z_ORDER + 505,
        playerAttackedData->xPosition + MINIMAP_X_POS,
        playerAttackedData->yPosition + MINIMAP_Y_POS,
        circleRadius, PAL_COLOR_VIOLET);
}


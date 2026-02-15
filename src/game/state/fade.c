#include "game/state/fade.h"
#include <allegro.h>

GameStateEnum handle_fade_in(GameContext *context, RenderQueue *renderQueue) {
    vsync();
    fade_in(context->mainPalette, context->fadeSpeed);
    context->ticksToCatchup = 0;
    timer_reset_ticks();
    return context->nextState;
}

GameStateEnum handle_fade_out(GameContext *context, RenderQueue *renderQueue) {
    vsync();
    fade_out(context->fadeSpeed);
    context->ticksToCatchup = 0;
    timer_reset_ticks();
    return context->nextState;
}

GameStateEnum fade_in_init(GameContext *context, GameStateEnum nextState, uint8_t speed) {
    context->nextState = nextState;
    context->fadeSpeed = speed;
    return GAME_STATE_FADE_IN;
}

GameStateEnum fade_out_init(GameContext *context, GameStateEnum nextState, uint8_t speed) {
    context->nextState = nextState;
    context->fadeSpeed = speed;
    return GAME_STATE_FADE_OUT;
}

#include "animation.h"

static AnimationSheetProperties IDLE_PROPERTIES[DIRECTIONS_COUNT] = {
		{.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
        {.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
        {.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
        {.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
        {.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
        {.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
        {.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
        {.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
};

static AnimationData WORKER_IDLE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_PINGPONG,
		.frameDuration = {0, 0, 0, 0},
		.numFrames = 4,
		.events = {{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_HUMAN_STEP, .fireTime = 80}},
		.numEvents = 1,
};

static Animation WORKER_IDLE[DIRECTIONS_COUNT] = {
		{.prop = &IDLE_PROPERTIES[DIRECTION_NORTH],         .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_NORTH_EAST],    .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_EAST],          .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_SOUTH_EAST],    .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_SOUTH],         .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_SOUTH_WEST],    .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_WEST],          .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_NORTH_WEST],    .data = &WORKER_IDLE_ANIMATION_DATA},
};

Animation* MOVABLE_UNIT_ANIMATIONS[MOVABLE_UNITS][MOVABLE_UNITS_STATES] = {
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_IDLE},
};

void game_animation_movable_unit_set(GameUnit* unit) {
    unit->animation = &(MOVABLE_UNIT_ANIMATIONS[unit->type][unit->state][unit->direction]);
    game_animation_unit_reset(unit);
}

void game_animation_unit_advance(GameUnit* unit) {
    // TODO advance animation and fire event if needed
}

uint8_t game_animation_unit_finished(GameUnit* unit) {
    AnimationData* data = unit->animation->data;
    // Infinite animations are always finished
    if(data->type != ANIMATION_TYPE_ONCE) return TRUE;
    uint8_t frame = clamp(unit->animationFrame, 0, data->numFrames);    
    return frame == data->numFrames && unit->animationFrameTicks >= data->frameDuration[frame];
}

void game_animation_unit_reset(GameUnit* unit) {
    unit->animationFrame = 0;
    unit->animationFrameTicks = 0;
    unit->animationTotalTicks = 0;
}

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

Animation* game_animation_movable_unit_get(GameUnit* unit) {
    return &(MOVABLE_UNIT_ANIMATIONS[unit->type][unit->state][unit->direction]);
}

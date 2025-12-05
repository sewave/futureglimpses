#include "animation.h"

#define SEC_TO_FRAMES(secs) (uint16_t) (secs * LOGIC_RATE_BPS)

static AnimationProperties IDLE_PROPERTIES[DIRECTIONS_COUNT] = {
		{.xOffset = 0, .yOffset = 320, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
		{.xOffset = 0, .yOffset = 320, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
		{.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
		{.xOffset = 0, .yOffset = 160, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
		{.xOffset = 0, .yOffset = 160, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
		{.xOffset = 0, .yOffset = 160, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
		{.xOffset = 0, .yOffset = 0, .width = 32, .height = 32, .hFlip = TRUE, .vFlip = FALSE},
		{.xOffset = 0, .yOffset = 320, .width = 32, .height = 32, .hFlip = FALSE, .vFlip = FALSE},
};

static AnimationData WORKER_IDLE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frameDuration = {SEC_TO_FRAMES(0.1), SEC_TO_FRAMES(0.1), SEC_TO_FRAMES(0.1), SEC_TO_FRAMES(0.1)},
		.lastFrameIndex = 3,
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

void game_animation_unit_set(GameUnit *unit) {
	unit->animationStatus.animation = &(MOVABLE_UNIT_ANIMATIONS[unit->type][unit->state][unit->direction]);
	game_animation_unit_reset(unit);
}

void game_animation_unit_advance(GameUnit* unit) {
    AnimationStatus* animationStatus = &unit->animationStatus;
    AnimationData *data = animationStatus->animation->data;
    uint16_t frameDuration = data->frameDuration[animationStatus->frame];
    if(animationStatus->frameTicks < frameDuration) {
        ++animationStatus->frameTicks;
        ++animationStatus->totalTicks;
        // TODO fire event/s if needed
        if(animationStatus->frameTicks == frameDuration) {
            if(animationStatus->frame == data->lastFrameIndex) {
                if(data->type == ANIMATION_TYPE_CYCLE) game_animation_unit_reset(unit);
            }
            else {
                ++animationStatus->frame;
                animationStatus->frameTicks = 0;
            }
        }
    }
}

uint8_t game_animation_unit_finished(GameUnit* unit) {
	AnimationData *data = unit->animationStatus.animation->data;
	// Infinite animations are always finished
    if(data->type != ANIMATION_TYPE_ONCE) return TRUE;
	uint8_t frame = clamp(unit->animationStatus.frame, 0, data->lastFrameIndex);
	return frame == data->lastFrameIndex && unit->animationStatus.frameTicks >= data->frameDuration[frame];
}

void game_animation_unit_reset(GameUnit* unit) {
	unit->animationStatus.frame = 0;
	unit->animationStatus.frameTicks = 0;
	unit->animationStatus.totalTicks = 0;
}

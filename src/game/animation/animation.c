#include "animation.h"

#define SEC_TO_FRAMES(secs) (uint16_t) (secs * LOGIC_RATE_BPS)
#define UNIT_FRAME_SIZE 32

static AnimationProperties IDLE_PROPERTIES[DIRECTIONS_COUNT] = {
		{.yOffset = 320, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		{.yOffset = 320, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		{.yOffset = 0,   .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		{.yOffset = 160, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		{.yOffset = 160, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		{.yOffset = 160, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		{.yOffset = 0,   .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		{.yOffset = 320, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
};

static AnimationData WORKER_IDLE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.2), .xOffset = 0},
            {.duration = SEC_TO_FRAMES(0.2), .xOffset = UNIT_FRAME_SIZE},
            {.duration = SEC_TO_FRAMES(0.2), .xOffset = UNIT_FRAME_SIZE * 2},
            {.duration = SEC_TO_FRAMES(0.2), .xOffset = UNIT_FRAME_SIZE * 3},
        },
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
    AnimationFrame* frame = &data->frames[animationStatus->frame];
    if(animationStatus->frameTicks < frame->duration) {
        ++animationStatus->frameTicks;
        ++animationStatus->totalTicks;
        // TODO fire event/s if needed
        if(animationStatus->frameTicks == frame->duration) {
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
	// Looping animations are always finished
    if(data->type != ANIMATION_TYPE_ONCE) return TRUE;
    uint8_t frame = unit->animationStatus.frame;
    AnimationFrame* frameAnim = &data->frames[frame];
	return frame == data->lastFrameIndex && unit->animationStatus.frameTicks >= frameAnim->duration;
}

void game_animation_unit_reset(GameUnit* unit) {
	unit->animationStatus.frame = 0;
	unit->animationStatus.frameTicks = 0;
	unit->animationStatus.totalTicks = 0;
}

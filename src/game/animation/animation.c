#include "animation.h"

#define SEC_TO_FRAMES(secs) (uint16_t) (secs * LOGIC_RATE_BPS)
#define UNIT_FRAME_SIZE 32

static AnimationProperties IDLE_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.yOffset = 320, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_EAST
		{.yOffset = 0,   .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_SOUTH
		{.yOffset = 160, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_WEST
		{.yOffset = 480, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
};

static AnimationProperties MOVE_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.yOffset = 352, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_EAST
		{.yOffset = 0,   .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_SOUTH
		{.yOffset = 192, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_WEST
		{.yOffset = 512, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
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

static AnimationData WORKER_MOVE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = 0},
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = UNIT_FRAME_SIZE},
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = UNIT_FRAME_SIZE * 2},
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = UNIT_FRAME_SIZE * 3},
        },
		.lastFrameIndex = 3,
		.events = {{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_HUMAN_STEP, .fireTime = 80}},
		.numEvents = 1,
};

static Animation WORKER_IDLE[DIRECTIONS_COUNT] = {
		{.prop = &IDLE_PROPERTIES[DIRECTION_NORTH], .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_EAST],  .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_SOUTH], .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_WEST],  .data = &WORKER_IDLE_ANIMATION_DATA},
};

static Animation WORKER_MOVE[DIRECTIONS_COUNT] = {
		{.prop = &MOVE_PROPERTIES[DIRECTION_NORTH], .data = &WORKER_MOVE_ANIMATION_DATA},
		{.prop = &MOVE_PROPERTIES[DIRECTION_EAST],  .data = &WORKER_MOVE_ANIMATION_DATA},
		{.prop = &MOVE_PROPERTIES[DIRECTION_SOUTH], .data = &WORKER_MOVE_ANIMATION_DATA},
		{.prop = &MOVE_PROPERTIES[DIRECTION_WEST],  .data = &WORKER_MOVE_ANIMATION_DATA},
};

// UNIT_STATE_IDLE, UNIT_STATE_ATTACK, UNIT_STATE_DEFEND, UNIT_STATE_MOVE, UNIT_STATE_MOVE_ANIM, UNIT_STATE_MOVE_ATTACK, UNIT_STATE_WORK
Animation* MOVABLE_UNIT_ANIMATIONS[MOVABLE_UNITS][UNIT_STATES_COUNT] = {
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_IDLE, WORKER_IDLE},
    {WORKER_IDLE, WORKER_IDLE, WORKER_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_IDLE, WORKER_IDLE},
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
                // TODO test to do all animations
                unit->state = (unit->state + 1) % UNIT_STATE_WORK;
                unit->direction = (unit->direction + 1) % DIRECTIONS_COUNT;
                game_animation_unit_set(unit);
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

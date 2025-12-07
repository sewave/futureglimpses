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
		{.yOffset = 32,  .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_SOUTH
		{.yOffset = 192, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_WEST
		{.yOffset = 512, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
};

static AnimationProperties ATTACK_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.yOffset = 384, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_EAST
		{.yOffset = 64,   .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_SOUTH
		{.yOffset = 224, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
        // DIRECTION_WEST
		{.yOffset = 544, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
};

static AnimationProperties DIE_PROPERTIES[DIRECTIONS_COUNT] = {
		// DIRECTION_NORTH
		{.yOffset = 416, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		// DIRECTION_EAST
		{.yOffset = 96, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		// DIRECTION_SOUTH
		{.yOffset = 256, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
		// DIRECTION_WEST
		{.yOffset = 576, .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
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
		.events = {},
		.numEvents = 0,
};

static AnimationData WORKER_MOVE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = 0},
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = UNIT_FRAME_SIZE},
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = UNIT_FRAME_SIZE * 2},
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = UNIT_FRAME_SIZE * 3},
        },
		.lastFrameIndex = 3,
		.events = {},
		.numEvents = 0,
};

static AnimationData WORKER_ATTACK_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.5), .xOffset = 0},
            {.duration = SEC_TO_FRAMES(0.1), .xOffset = UNIT_FRAME_SIZE},
            {.duration = SEC_TO_FRAMES(0.3), .xOffset = UNIT_FRAME_SIZE * 2},
            {.duration = SEC_TO_FRAMES(0.3), .xOffset = UNIT_FRAME_SIZE * 3},
        },
		.lastFrameIndex = 3,
		.events = { {.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_HIT, .fireTime = SEC_TO_FRAMES(0.7)},
                    {.type = EVENT_TYPE_DAMAGE, .data = 0, .fireTime = SEC_TO_FRAMES(0.8)}},
		.numEvents = 2,
};

static AnimationData COMMON_DIE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
				{.duration = SEC_TO_FRAMES(0.2), .xOffset = 0},
				{.duration = SEC_TO_FRAMES(0.2), .xOffset = UNIT_FRAME_SIZE},
				{.duration = SEC_TO_FRAMES(0.2), .xOffset = UNIT_FRAME_SIZE * 2},
				{.duration = SEC_TO_FRAMES(0.5), .xOffset = UNIT_FRAME_SIZE * 3},
		},
		.lastFrameIndex = 3,
		.events = {{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_DIE, .fireTime = SEC_TO_FRAMES(0.3)}},
		.numEvents = 1,
};

static Animation COMMON_IDLE[DIRECTIONS_COUNT] = {
		{.prop = &IDLE_PROPERTIES[DIRECTION_NORTH], .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_EAST], .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_SOUTH], .data = &WORKER_IDLE_ANIMATION_DATA},
		{.prop = &IDLE_PROPERTIES[DIRECTION_WEST], .data = &WORKER_IDLE_ANIMATION_DATA},
};

static Animation WORKER_MOVE[DIRECTIONS_COUNT] = {
		{.prop = &MOVE_PROPERTIES[DIRECTION_NORTH], .data = &WORKER_MOVE_ANIMATION_DATA},
		{.prop = &MOVE_PROPERTIES[DIRECTION_EAST],  .data = &WORKER_MOVE_ANIMATION_DATA},
		{.prop = &MOVE_PROPERTIES[DIRECTION_SOUTH], .data = &WORKER_MOVE_ANIMATION_DATA},
		{.prop = &MOVE_PROPERTIES[DIRECTION_WEST],  .data = &WORKER_MOVE_ANIMATION_DATA},
};

static Animation WORKER_ATTACK[DIRECTIONS_COUNT] = {
		{.prop = &ATTACK_PROPERTIES[DIRECTION_NORTH], .data = &WORKER_ATTACK_ANIMATION_DATA},
		{.prop = &ATTACK_PROPERTIES[DIRECTION_EAST],  .data = &WORKER_ATTACK_ANIMATION_DATA},
		{.prop = &ATTACK_PROPERTIES[DIRECTION_SOUTH], .data = &WORKER_ATTACK_ANIMATION_DATA},
		{.prop = &ATTACK_PROPERTIES[DIRECTION_WEST],  .data = &WORKER_ATTACK_ANIMATION_DATA},
};

static Animation COMMON_DIE[DIRECTIONS_COUNT] = {
		{.prop = &DIE_PROPERTIES[DIRECTION_NORTH], .data = &COMMON_DIE_ANIMATION_DATA},
		{.prop = &DIE_PROPERTIES[DIRECTION_EAST], .data = &COMMON_DIE_ANIMATION_DATA},
		{.prop = &DIE_PROPERTIES[DIRECTION_SOUTH], .data = &COMMON_DIE_ANIMATION_DATA},
		{.prop = &DIE_PROPERTIES[DIRECTION_WEST], .data = &COMMON_DIE_ANIMATION_DATA},
};

// UNIT_STATE_IDLE, UNIT_STATE_ATTACK, UNIT_STATE_DEFEND, UNIT_STATE_MOVE, UNIT_STATE_MOVE_ANIM, UNIT_STATE_MOVE_ATTACK, UNIT_STATE_WORK
Animation *MOVABLE_UNIT_ANIMATIONS[MOVABLE_UNITS][UNIT_STATES_COUNT] = {
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
};

void game_animation_unit_set(GameUnit *unit) {
	unit->animationStatus.animation = &(MOVABLE_UNIT_ANIMATIONS[unit->type][unit->state][unit->direction]);
	game_animation_unit_reset(unit);
}

void game_animation_unit_advance(GameContext* context, GameUnit* unit) {
    AnimationStatus* animationStatus = &unit->animationStatus;
    AnimationData *data = animationStatus->animation->data;
    AnimationFrame* frame = &data->frames[animationStatus->frame];
    if(animationStatus->frameTicks < frame->duration) {
        ++animationStatus->frameTicks;
        ++animationStatus->totalTicks;
        AnimationEvent* event = data->events;
        for(uint8_t i = 0; i < data->numEvents; i++, event++) {
            if(event->fireTime == animationStatus->totalTicks) {
                game_event_process(context, event->type, unit, event->data);
            }
        }
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

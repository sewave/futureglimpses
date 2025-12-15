#include "animation.h"

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
		{.yOffset = 64,  .xRepos = 8, .yRepos = 8, .width = UNIT_FRAME_SIZE, .height = UNIT_FRAME_SIZE},
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

typedef struct {
    AnimationProperties* props;
    AnimationData* data;
} AnimationPropsData;

static const AnimationPropsData COMMON_IDLE = {
    .props = IDLE_PROPERTIES,
    .data = &WORKER_IDLE_ANIMATION_DATA,
};

static const AnimationPropsData WORKER_MOVE = {
    .props = MOVE_PROPERTIES,
    .data = &WORKER_MOVE_ANIMATION_DATA,
};

static const AnimationPropsData WORKER_ATTACK = {
    .props = ATTACK_PROPERTIES,
    .data = &WORKER_ATTACK_ANIMATION_DATA,
};

static const AnimationPropsData COMMON_DIE = {
    .props = DIE_PROPERTIES,
    .data = &COMMON_DIE_ANIMATION_DATA,
};

// UNIT_STATE_IDLE, UNIT_STATE_ATTACK, UNIT_STATE_DEFEND, UNIT_STATE_MOVE, UNIT_STATE_MOVE_ANIM, UNIT_STATE_MOVE_ATTACK, UNIT_STATE_WORK
AnimationPropsData MOVABLE_UNIT_ANIMATIONS[MOVABLE_UNITS][UNIT_STATES_COUNT] = {
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		{COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
};

static AnimationProperties ARROW_PROPERTIES[DIRECTIONS_COUNT] = {
		// DIRECTION_NORTH
		{.yOffset = 0, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
		// DIRECTION_EAST
		{.yOffset = 16, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
		// DIRECTION_SOUTH
		{.yOffset = 32, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
		// DIRECTION_WEST
		{.yOffset = 48, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
};

static AnimationProperties FIREBALL_PROPERTIES[DIRECTIONS_COUNT] = {
		// DIRECTION_NORTH
		{.yOffset = 0, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
		// DIRECTION_EAST
		{.yOffset = 16, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
		// DIRECTION_SOUTH
		{.yOffset = 32, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
		// DIRECTION_WEST
		{.yOffset = 48, .xRepos = 0, .yRepos = 0, .width = 16, .height = 16},
};

static AnimationProperties EXPLOSION_PROPERTIES[DIRECTIONS_COUNT] = {
		// DIRECTION_NORTH
		{.yOffset = 0, .xRepos = 16, .yRepos = 16, .width = 48, .height = 48},
		// DIRECTION_EAST
		{.yOffset = 0, .xRepos = 16, .yRepos = 16, .width = 48, .height = 48},
		// DIRECTION_SOUTH
		{.yOffset = 0, .xRepos = 16, .yRepos = 16, .width = 48, .height = 48},
		// DIRECTION_WEST
		{.yOffset = 0, .xRepos = 16, .yRepos = 16, .width = 48, .height = 48},
};

static AnimationData ARROW_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
				{.duration = SEC_TO_FRAMES(5.0), .xOffset = 0},
		},
		.lastFrameIndex = 0,
        // TODO arrow hit sound
		.events = {{.type = EVENT_TYPE_DAMAGE, .data = 0, .fireTime = SEC_TO_FRAMES(0.5)}},
		.numEvents = 1,
};

static AnimationData FIREBALL_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
				{.duration = SEC_TO_FRAMES(5.0), .xOffset = 0},
		},
		.lastFrameIndex = 0,
        // TODO explosion sound
		.events = {},
		.numEvents = 0,
};

static AnimationData EXPLOSION_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
				{.duration = SEC_TO_FRAMES(0.2), .xOffset = 0},
                {.duration = SEC_TO_FRAMES(0.2), .xOffset = 48},
                {.duration = SEC_TO_FRAMES(0.2), .xOffset = 96},
                {.duration = SEC_TO_FRAMES(0.2), .xOffset = 144},
		},
		.lastFrameIndex = 3,
		.events = {{.type = EVENT_TYPE_AREA_DAMAGE, .data = 0, .fireTime = SEC_TO_FRAMES(0.1)}},
		.numEvents = 1,
};

static const AnimationPropsData ARROW_ANIM = {
    .props = ARROW_PROPERTIES,
    .data = &ARROW_ANIMATION_DATA,
};

static const AnimationPropsData FIREBALL_ANIM = {
    .props = FIREBALL_PROPERTIES,
    .data = &FIREBALL_ANIMATION_DATA,
};

static const AnimationPropsData EXPLOSION_ANIM = {
    .props = EXPLOSION_PROPERTIES,
    .data = &EXPLOSION_ANIMATION_DATA,
};

AnimationPropsData OBJECT_ANIMATIONS[OBJ_TYPE_NUMBER] = {
        ARROW_ANIM, FIREBALL_ANIM, EXPLOSION_ANIM
};

void game_animation_unit_set(GameUnit *unit) {
    AnimationPropsData propsData = MOVABLE_UNIT_ANIMATIONS[unit->type][unit->state];
    AnimationStatus* animationStatus = &unit->animationStatus;
	animationStatus->animation.prop = &propsData.props[unit->direction];
    animationStatus->animation.data = propsData.data;
	game_animation_reset(animationStatus);
}

void game_animation_unit_advance(GameContext* context, GameUnit* unit) {
    if(unit->blinkTime) --unit->blinkTime;
    AnimationStatus* animationStatus = &unit->animationStatus;
    AnimationData *data = animationStatus->animation.data;
    AnimationFrame* frame = &data->frames[animationStatus->frame];
    if(animationStatus->frameTicks < frame->duration) {
        ++animationStatus->frameTicks;
        ++animationStatus->totalTicks;
        AnimationEvent* event = data->events;
        for(uint8_t i = 0; i < data->numEvents; i++, event++) {
            if(event->fireTime == animationStatus->totalTicks) {
				game_event_unit_process(context, event->type, unit, event->data);
			}
        }
        if(animationStatus->frameTicks == frame->duration) {
            if(animationStatus->frame == data->lastFrameIndex) {
				if (data->type == ANIMATION_TYPE_CYCLE) game_animation_reset(animationStatus);
			}
            else {
                ++animationStatus->frame;
                animationStatus->frameTicks = 0;
            }
        }
    }
}

void game_animation_object_advance(GameContext* context, Object* object) {
    // TODO how to join with unit animation advance?
    AnimationStatus* animationStatus = &object->animationStatus;
    AnimationData *data = animationStatus->animation.data;
    AnimationFrame* frame = &data->frames[animationStatus->frame];
    if(animationStatus->frameTicks < frame->duration) {
        ++animationStatus->frameTicks;
        ++animationStatus->totalTicks;
        AnimationEvent* event = data->events;
        for(uint8_t i = 0; i < data->numEvents; i++, event++) {
            if(event->fireTime == animationStatus->totalTicks) {
				game_event_object_process(context, event->type, object, event->data);
			}
        }
        if(animationStatus->frameTicks == frame->duration) {
            if(animationStatus->frame == data->lastFrameIndex) {
				if (data->type == ANIMATION_TYPE_CYCLE) game_animation_reset(animationStatus);
			}
            else {
                ++animationStatus->frame;
                animationStatus->frameTicks = 0;
            }
        }
    }
}

uint8_t game_animation_finished(AnimationStatus* animationStatus) {
	AnimationData *data = animationStatus->animation.data;
	// Looping animations are always finished
    if(data->type != ANIMATION_TYPE_ONCE) return TRUE;
    uint8_t frame = animationStatus->frame;
    AnimationFrame* frameAnim = &data->frames[frame];
	return frame == data->lastFrameIndex && animationStatus->frameTicks >= frameAnim->duration;
}

void game_animation_reset(AnimationStatus* animationStatus) {
	animationStatus->frame = 0;
	animationStatus->frameTicks = 0;
	animationStatus->totalTicks = 0;
}

void game_animation_object_set(Object *object) {
    AnimationPropsData propsData = OBJECT_ANIMATIONS[object->type];
    AnimationStatus* animationStatus = &object->animationStatus;
	animationStatus->animation.prop = &propsData.props[object->direction];
    animationStatus->animation.data = propsData.data;
	game_animation_reset(animationStatus);
}

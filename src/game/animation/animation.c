#include "game/animation/animation.h"
#include "game/sound/game_sound.h"
#include "game/event/event.h"

#define UNIT_ANI_FRAMES 4

static AnimationProperties IDLE_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.startFrame = 8 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_EAST
		{.startFrame = 0 * UNIT_ANI_FRAMES,	.xRepos = 8, .yRepos = 8},
        // DIRECTION_SOUTH
		{.startFrame = 4 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_WEST
		{.startFrame = 12 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
};

static AnimationProperties MOVE_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.startFrame = 9 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_EAST
		{.startFrame = 1 * UNIT_ANI_FRAMES,	.xRepos = 8, .yRepos = 8},
        // DIRECTION_SOUTH
		{.startFrame = 5 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_WEST
		{.startFrame = 13 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
};

static AnimationProperties ATTACK_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.startFrame = 10 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_EAST
		{.startFrame = 2 * UNIT_ANI_FRAMES,	.xRepos = 8, .yRepos = 8},
        // DIRECTION_SOUTH
		{.startFrame = 6 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_WEST
		{.startFrame = 14 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
};

static AnimationProperties DIE_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.startFrame = 11 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_EAST
		{.startFrame = 3 * UNIT_ANI_FRAMES,	.xRepos = 8, .yRepos = 8},
        // DIRECTION_SOUTH
		{.startFrame = 7 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
        // DIRECTION_WEST
		{.startFrame = 15 * UNIT_ANI_FRAMES, .xRepos = 8, .yRepos = 8},
};

static AnimationData WORKER_IDLE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.2)},
            {.duration = SEC_TO_FRAMES(0.2)},
            {.duration = SEC_TO_FRAMES(0.2)},
            {.duration = SEC_TO_FRAMES(0.2)},
        },
		.lastFrameIndex = 3,
		.events = {},
		.numEvents = 0,
};

static AnimationData WORKER_MOVE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.1)},
            {.duration = SEC_TO_FRAMES(0.1)},
            {.duration = SEC_TO_FRAMES(0.1)},
            {.duration = SEC_TO_FRAMES(0.1)},
        },
		.lastFrameIndex = 3,
		.events = {},
		.numEvents = 0,
};

static AnimationData WORKER_ATTACK_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.5)},
            {.duration = SEC_TO_FRAMES(0.1)},
            {.duration = SEC_TO_FRAMES(0.3)},
            {.duration = SEC_TO_FRAMES(0.3)},
        },
		.lastFrameIndex = 3,
		.events = { {.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_IRON_HIT, .fireTime = SEC_TO_FRAMES(0.7)},
                    {.type = EVENT_TYPE_DAMAGE, .data = 0, .fireTime = SEC_TO_FRAMES(0.8)}},
		.numEvents = 2,
};

static AnimationData WORKER_WORK_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.3)},
            {.duration = SEC_TO_FRAMES(0.1)},
            {.duration = SEC_TO_FRAMES(0.2)},
            {.duration = SEC_TO_FRAMES(0.2)},
        },
		.lastFrameIndex = 3,
		.events = {
			{.type = EVENT_TYPE_WORK_SOUND, .data = 0, .fireTime = SEC_TO_FRAMES(0.4)},
			{.type = EVENT_TYPE_WORK, .data = 0, .fireTime = WORKER_TIME},
		},
		.numEvents = 2,
};

static AnimationData ARCHER_ATTACK_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
				{.duration = SEC_TO_FRAMES(0.5)},
				{.duration = SEC_TO_FRAMES(0.1)},
				{.duration = SEC_TO_FRAMES(0.2)},
				{.duration = SEC_TO_FRAMES(0.2)},
		},
		.lastFrameIndex = 3,
		.events = {
				{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_THROW_ARROW, .fireTime = SEC_TO_FRAMES(0.5)},
				{.type = EVENT_TYPE_SPAWN_ARROW, .data = 0, .fireTime = SEC_TO_FRAMES(0.6)}
		},
		.numEvents = 2,
};

static AnimationData MAGE_ATTACK_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.6)},
            {.duration = SEC_TO_FRAMES(1)},
            {.duration = SEC_TO_FRAMES(0.3)},
            {.duration = SEC_TO_FRAMES(0.5)},
        },
		.lastFrameIndex = 3,
		.events = {
			{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_FIREBALL_LAUNCH, .fireTime = SEC_TO_FRAMES(1.9)},
			{.type = EVENT_TYPE_SPAWN_FIREBALL, .data = 0, .fireTime = SEC_TO_FRAMES(1.9)}},
		.numEvents = 2,
};

static AnimationData COMMON_DIE_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
				{.duration = SEC_TO_FRAMES(0.2)},
				{.duration = SEC_TO_FRAMES(0.2)},
				{.duration = SEC_TO_FRAMES(0.2)},
				{.duration = SEC_TO_FRAMES(0.5)},
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

static const AnimationPropsData WORKER_WORK = {
    .props = ATTACK_PROPERTIES,
    .data = &WORKER_WORK_ANIMATION_DATA,
};

static const AnimationPropsData ARCHER_ATTACK = {
    .props = ATTACK_PROPERTIES,
    .data = &ARCHER_ATTACK_ANIMATION_DATA,
};

static const AnimationPropsData MAGE_ATTACK = {
    .props = ATTACK_PROPERTIES,
    .data = &MAGE_ATTACK_ANIMATION_DATA,
};

static const AnimationPropsData COMMON_DIE = {
    .props = DIE_PROPERTIES,
    .data = &COMMON_DIE_ANIMATION_DATA,
};

static AnimationProperties BIG_BUILDING_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_EAST
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_SOUTH
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_WEST
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
};

static AnimationProperties SMALL_BUILDING_PROPERTIES[DIRECTIONS_COUNT] = {
        // DIRECTION_NORTH
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_EAST
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_SOUTH
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_WEST
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
};

static AnimationData BUILDING_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
            {.duration = SEC_TO_FRAMES(60)},
        },
		.lastFrameIndex = 0,
		.events = {},
		.numEvents = 0,
};

static AnimationData BUILDING_DESTROY_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
            {.duration = SEC_TO_FRAMES(0.5)},
        },
		.lastFrameIndex = 0,
		.events = {
			{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_BUILDING_CRUMBLE, .fireTime = SEC_TO_FRAMES(0.1)},
		},
		.numEvents = 1,
};

static const AnimationPropsData B_BUILD_IDLE = {
    .props = BIG_BUILDING_PROPERTIES,
    .data = &BUILDING_ANIMATION_DATA,
};

static const AnimationPropsData S_BUILD_IDLE = {
    .props = SMALL_BUILDING_PROPERTIES,
    .data = &BUILDING_ANIMATION_DATA,
};

static AnimationData TURRET_ATTACK_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
				{.duration = SEC_TO_FRAMES(1.0)},
		},
		.lastFrameIndex = 0,
		.events = {
				{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_THROW_ARROW, .fireTime = SEC_TO_FRAMES(0.5)},
				{.type = EVENT_TYPE_SPAWN_ARROW, .data = 0, .fireTime = SEC_TO_FRAMES(0.6)}
		},
		.numEvents = 2,
};

static const AnimationPropsData TURRET_ATTACK = {
    .props = SMALL_BUILDING_PROPERTIES,
    .data = &TURRET_ATTACK_ANIMATION_DATA,
};

static const AnimationPropsData B_BUILD_DESTROY = {
    .props = BIG_BUILDING_PROPERTIES,
    .data = &BUILDING_DESTROY_ANIMATION_DATA,
};

static const AnimationPropsData S_BUILD_DESTROY = {
    .props = SMALL_BUILDING_PROPERTIES,
    .data = &BUILDING_DESTROY_ANIMATION_DATA,
};

// UNIT_STATE_IDLE, UNIT_STATE_ATTACK, UNIT_STATE_DEFEND, UNIT_STATE_MOVE, UNIT_STATE_MOVE_ANIM, UNIT_STATE_MOVE_ATTACK, UNIT_STATE_WORK
static AnimationPropsData UNIT_ANIMATIONS[UNIT_TYPE_NUMBER][UNIT_STATES_COUNT] = {
		[UNIT_TYPE_WORKER] = {COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_WORK, COMMON_DIE},
		[UNIT_TYPE_SOLDIER] = {COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		[UNIT_TYPE_ARCHER] = {COMMON_IDLE, ARCHER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		[UNIT_TYPE_KNIGHT] = {COMMON_IDLE, WORKER_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		[UNIT_TYPE_MAGE] = {COMMON_IDLE, MAGE_ATTACK, COMMON_IDLE, WORKER_MOVE, WORKER_MOVE, WORKER_MOVE, WORKER_ATTACK, COMMON_DIE},
		[UNIT_TYPE_CITY_HALL] = {B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_DESTROY},
		[UNIT_TYPE_FARM] = {S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_DESTROY},
		[UNIT_TYPE_BARRACKS] = {B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_DESTROY},
		[UNIT_TYPE_BLACKSMITH] = {S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_DESTROY},
		[UNIT_TYPE_STABLES] = {B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_IDLE, B_BUILD_DESTROY},
		[UNIT_TYPE_TOWER] = {S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_DESTROY},
		[UNIT_TYPE_TURRET] = {S_BUILD_IDLE, TURRET_ATTACK, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_IDLE, S_BUILD_DESTROY},
};

static AnimationProperties ARROW_PROPERTIES[OBJ_DIRECTIONS_COUNT] = {
		// DIRECTION_NORTH
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_NORTH_EAST
		{.startFrame = 1, .xRepos = 0, .yRepos = 0},
		// DIRECTION_EAST
		{.startFrame = 2, .xRepos = 0, .yRepos = 0},
		// DIRECTION_SOUTH_EAST
		{.startFrame = 3, .xRepos = 0, .yRepos = 0},
		// DIRECTION_SOUTH
		{.startFrame = 4, .xRepos = 0, .yRepos = 0},
		// DIRECTION_SOUTH_WEST
		{.startFrame = 5, .xRepos = 0, .yRepos = 0},
		// DIRECTION_WEST
		{.startFrame = 6, .xRepos = 0, .yRepos = 0},
		// DIRECTION_NORTH_WEST
		{.startFrame = 7, .xRepos = 0, .yRepos = 0},
};

static AnimationProperties FIREBALL_PROPERTIES[OBJ_DIRECTIONS_COUNT] = {
		// DIRECTION_NORTH
		{.startFrame = 0, .xRepos = 0, .yRepos = 0},
        // DIRECTION_NORTH_EAST
		{.startFrame = 1, .xRepos = 0, .yRepos = 0},
		// DIRECTION_EAST
		{.startFrame = 2, .xRepos = 0, .yRepos = 0},
		// DIRECTION_SOUTH_EAST
		{.startFrame = 3, .xRepos = 0, .yRepos = 0},
		// DIRECTION_SOUTH
		{.startFrame = 4, .xRepos = 0, .yRepos = 0},
		// DIRECTION_SOUTH_WEST
		{.startFrame = 5, .xRepos = 0, .yRepos = 0},
		// DIRECTION_WEST
		{.startFrame = 6, .xRepos = 0, .yRepos = 0},
		// DIRECTION_NORTH_WEST
		{.startFrame = 7, .xRepos = 0, .yRepos = 0},
};

static AnimationProperties EXPLOSION_PROPERTIES[OBJ_DIRECTIONS_COUNT] = {
		// DIRECTION_NORTH
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
        // DIRECTION_NORTH_EAST
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
		// DIRECTION_EAST
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
		// DIRECTION_SOUTH_EAST
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
		// DIRECTION_SOUTH
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
		// DIRECTION_SOUTH_WEST
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
		// DIRECTION_WEST
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
		// DIRECTION_NORTH_WEST
		{.startFrame = 0, .xRepos = 16, .yRepos = 16},
};

static AnimationData ARROW_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
				{.duration = SEC_TO_FRAMES(5.0)},
		},
		.lastFrameIndex = 0,
		.events = { },
		.numEvents = 0,
};

static AnimationData FIREBALL_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_CYCLE,
		.frames = {
				{.duration = SEC_TO_FRAMES(5.0)},
		},
		.lastFrameIndex = 0,
		.events = {},
		.numEvents = 0,
};

static AnimationData EXPLOSION_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
				{.duration = SEC_TO_FRAMES(0.2)},
                {.duration = SEC_TO_FRAMES(0.2)},
                {.duration = SEC_TO_FRAMES(0.2)},
                {.duration = SEC_TO_FRAMES(0.2)},
		},
		.lastFrameIndex = 3,
		.events = {
            {.type = EVENT_TYPE_AREA_DAMAGE, .data = 0, .fireTime = SEC_TO_FRAMES(0.1)},
			{.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_FIREBALL_EXPLOSION, .fireTime = SEC_TO_FRAMES(0.1)}
        },
		.numEvents = 2,
};

static AnimationData ARROW_HIT_ANIMATION_DATA = {
		.type = ANIMATION_TYPE_ONCE,
		.frames = {
				{.duration = SEC_TO_FRAMES(0.15)},
		},
		.lastFrameIndex = 0,
		.events = {
            {.type = EVENT_TYPE_SOUND, .data = GAME_SOUND_ARROW_HIT, .fireTime = SEC_TO_FRAMES(0.1)},
            {.type = EVENT_TYPE_DAMAGE, .data = 0, .fireTime = SEC_TO_FRAMES(0.1)}},
		.numEvents = 2,
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

static const AnimationPropsData ARROW_DAMAGE_ANIM = {
	.props = ARROW_PROPERTIES,
	.data = &ARROW_HIT_ANIMATION_DATA,
};

AnimationPropsData OBJECT_ANIMATIONS[OBJ_TYPE_NUMBER] = {
        ARROW_ANIM, FIREBALL_ANIM, EXPLOSION_ANIM, ARROW_DAMAGE_ANIM
};

// Common animation advancement logic
typedef void (*AnimationEventProcessor)(GameContext *context, AnimationEvent *event, void *entity);

static void game_animation_advance_internal(GameContext *context, AnimationStatus *animationStatus,
											AnimationEventProcessor eventProcessor, void *entity) {
	AnimationData *data = animationStatus->animation.data;
	AnimationFrame *frame = &data->frames[animationStatus->frame];
	if (animationStatus->frameTicks < frame->duration) {
		++animationStatus->frameTicks;
		++animationStatus->totalTicks;
		AnimationEvent *event = data->events;
		for (uint8_t i = 0; i < data->numEvents; i++, event++) {
			if (event->fireTime == animationStatus->totalTicks) {
				eventProcessor(context, event, entity);
			}
		}
		if (animationStatus->frameTicks == frame->duration) {
			if (animationStatus->frame == data->lastFrameIndex) {
				if (data->type == ANIMATION_TYPE_CYCLE) game_animation_reset(animationStatus);
			} else {
				++animationStatus->frame;
				animationStatus->frameTicks = 0;
			}
		}
	}
}

static void game_animation_unit_event_processor(GameContext *context, AnimationEvent *event, void *entity) {
	game_event_unit_process(context, event->type, (GameUnit *) entity, event->data);
}

static void game_animation_object_event_processor(GameContext *context, AnimationEvent *event, void *entity) {
	game_event_object_process(context, event->type, (Object *) entity, event->data);
}

void game_animation_unit_set(GameUnit *unit) {
    AnimationPropsData* propsData = &UNIT_ANIMATIONS[unit->type][unit->state];
    AnimationStatus* animationStatus = &unit->animationStatus;
	animationStatus->animation.prop = &propsData->props[unit->direction];
    animationStatus->animation.data = propsData->data;
	game_animation_reset(animationStatus);
}

void game_animation_unit_advance(GameContext *context, GameUnit *unit) {
	if (unit->blinkTime) --unit->blinkTime;
	game_animation_advance_internal(context, &unit->animationStatus, game_animation_unit_event_processor, unit);
}

void game_animation_object_advance(GameContext *context, Object *object) {
	game_animation_advance_internal(context, &object->animationStatus, game_animation_object_event_processor, object);
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

uint16_t game_animation_unit_get_frame_position(UnitTypeEnum type, UnitStateEnum state, DirectionEnum direction, uint8_t frame) {
	return UNIT_ANIMATIONS[type][state].props[direction].startFrame + frame;
}

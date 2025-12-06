#ifndef GAME_H
#define GAME_H
#include "../common/common_lib.h"
#include <allegro/gfx.h>

#define BOARD_WIDTH 64
#define BOARD_HEIGHT 64
#define TILE_SIZE 16
#define BOARD_SIZE (BOARD_WIDTH * BOARD_HEIGHT)
#define WORLD_WIDTH BOARD_WIDTH *TILE_SIZE
#define WORLD_HEIGHT BOARD_HEIGHT *TILE_SIZE
#define INVERSE_TILE_SIZE 1.0f / TILE_SIZE

#define UNUSED_BUCKET_ID -1
#define FREE_UNIT_SLOT_NOT_FOUND -1
#define NO_TARGET_POSITION -1
#define NO_TARGET_ID -1

// --- ID GENERATION SETTINGS (16 bits index, 16 bits generation) ---
#define ID_INDEX_MASK 0xFFFF// Lower 16 bits for Index
#define ID_GEN_SHIFT 16     // Upper 16 bits for Generation

#define GET_INDEX(id) (id & ID_INDEX_MASK)
#define GET_GEN(id) ((id >> ID_GEN_SHIFT) & 0xFFFF)
#define MAKE_ID(index, gen) ((gen << ID_GEN_SHIFT) | (index & ID_INDEX_MASK))
#define NULL_HANDLE 0

typedef struct {
	int startCol;
	int endCol;
	int startRow;
	int endRow;
} GridRect;

typedef enum {
	GAME_STATE_LOAD_MAP,
	GAME_STATE_PLAY_MAP,
	GAME_STATE_EXIT,
	NUM_GAME_STATES
} GameStateEnum;

typedef enum {
	BOARD_UNEXPLORED,
	BOARD_EXPLORED
} BoardExplorationEnum;

typedef enum {
	WALKABILITY_FREE,
	WALKABILITY_BLOCKED,
} WalkabilityEnum;

typedef enum {
	UNIT_TYPE_WORKER,
	UNIT_TYPE_SOLDIER,
	UNIT_TYPE_ARCHER,
	UNIT_TYPE_MOUNT,
	UNIT_TYPE_MAGE,
	UNIT_TYPE_CITY_HALL,
	UNIT_TYPE_FARM,
	UNIT_TYPE_BARRACKS,
	UNIT_TYPE_BLACKSMITH,
	UNIT_TYPE_STABLES,
	UNIT_TYPE_TOWER,
	UNIT_TYPE_NUMBER,
} UnitTypeEnum;

typedef enum {
	UNIT_CONTROLLER_PLAYER,
	UNIT_CONTROLLER_AI
} UnitControllerEnum;

typedef enum {
	UNIT_STATE_IDLE,
	UNIT_STATE_ATTACK,
	UNIT_STATE_DEFEND,
	UNIT_STATE_MOVE,
	UNIT_STATE_MOVE_ANIM,
	UNIT_STATE_MOVE_ATTACK,
	UNIT_STATE_WORK,
	UNIT_STATES_COUNT,
} UnitStateEnum;

typedef enum {
	DIRECTION_NORTH,
	DIRECTION_NORTH_EAST,
	DIRECTION_EAST,
	DIRECTION_SOUTH_EAST,
	DIRECTION_SOUTH,
	DIRECTION_SOUTH_WEST,
	DIRECTION_WEST,
	DIRECTION_NORTH_WEST,
	DIRECTIONS_COUNT,
} DirectionEnum;

typedef enum {
	EVENT_TYPE_SOUND,
	EVENT_TYPE_DAMAGE,
	EVENT_TYPE_SPAWN_ARROW,
	EVENT_TYPE_SPAWN_MAGIC,
	EVENT_TYPE_WORK,
} EventType;

#define MAX_FRAMES 4
#define MAX_EVENTS 4

typedef struct {
	uint16_t yOffset;
	uint16_t width, height;
} AnimationProperties;

typedef enum {
	ANIMATION_TYPE_ONCE,
	ANIMATION_TYPE_CYCLE,
} AnimationType;

typedef struct {
	EventType type;
	uint16_t data;
	uint16_t fireTime;
} AnimationEvent;

typedef struct {
	uint16_t duration;
	uint16_t xOffset;
} AnimationFrame;

typedef struct {
	AnimationType type;
	AnimationFrame frames[MAX_FRAMES];
	uint8_t lastFrameIndex;
	AnimationEvent events[MAX_EVENTS];
	uint8_t numEvents;
} AnimationData;

typedef struct {
	AnimationProperties *prop;
	AnimationData* data;
} Animation;

typedef struct {
	BITMAP *sheet;
	Animation *animation;
	uint16_t frameTicks;
	uint16_t totalTicks;
	uint8_t frame;
} AnimationStatus;

#define MAX_GAME_UNITS 512

typedef uint32_t UnitId;

typedef struct {
	UnitId id;
	unsigned char isActive;
	UnitTypeEnum type;
	UnitControllerEnum controller;
	UnitStateEnum state;
	UnitStateEnum nextState;
	DirectionEnum direction;

	uint16_t x, y;
	uint8_t attackRange, sightRange;
	uint16_t health, maxHealth;
	uint8_t tileSize;

	uint16_t targetX, targetY;
	UnitId targetId;

	uint16_t reactionTime;
	uint16_t reactionTimeCounter;
	uint16_t moveTime;
	uint16_t moveTimeCounter;
	AnimationStatus animationStatus;
} GameUnit;

typedef struct {
	GameStateEnum gameState;
	BoardExplorationEnum boardExploration[BOARD_WIDTH][BOARD_HEIGHT];
	UnitId walkabilityGrid[BOARD_WIDTH][BOARD_HEIGHT];
	// TODO resources/walls grid
	int board[BOARD_WIDTH][BOARD_HEIGHT];
	GameUnit units[MAX_GAME_UNITS];
	// Whenever the board is modified, we re-render the modified parts to this bitmap
	BITMAP *renderedBoard;
	BITMAP *renderedMinimap;
	int xPosition, yPosition;// Top-left position of the viewport on the board
	BITMAP *gameBack;
	BITMAP *tileSet;
	long ticksToCatchup;
} GameContext;

typedef GameStateEnum (*StateFunction)(GameContext *, RenderQueue *);

void game_free_context(GameContext *context);
GameStateEnum game_execute_state(GameContext *context, RenderQueue * renderQueue);

#define LOGIC_RATE_BPS 60
#define MAX_CATCHUP_TICKS 5
// First MB is special so we check for 7 more
#define PROGRAM_REQUIRED_RAM_MB 7
#define MINIMAL_CPU_FAMILY CPU_FAMILY_I486
#define REQUIRED_CPU_CAPABILITIES CPU_FPU
#define UNSUPPORTED_CPU_MESSAGE "Error: CPU not supported. A 486 or better with FPU is required."

#endif /* GAME_H */

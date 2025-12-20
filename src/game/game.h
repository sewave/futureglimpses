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
#define BOARD_X_MIN 0
#define BOARD_Y_MIN 0
#define BOARD_X_MAX BOARD_WIDTH - 1
#define BOARD_Y_MAX BOARD_HEIGHT - 1

#define NO_TARGET_POSITION 65535
#define NO_TARGET_ID 0

// --- ID GENERATION SETTINGS (16 bits index, 16 bits generation) ---
#define ID_INDEX_MASK 0xFFFF// Lower 16 bits for Index
#define ID_GEN_SHIFT 16     // Upper 16 bits for Generation
#define HANDLE_ID_THRESHOLD (1 << ID_GEN_SHIFT)

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
	RESOURCE_TYPE_GOLD,
	RESOURCE_TYPE_WOOD,
	RESOURCE_TYPES_COUNT
} ResourceTypeEnum;

typedef struct {
	uint32_t quantity[RESOURCE_TYPES_COUNT];
	uint32_t uiQuantity[RESOURCE_TYPES_COUNT];
} Resources;

typedef struct {
	uint32_t unitsTrained;
	uint32_t enemiesKilled;
	uint32_t buildingsConstructed;
	uint32_t buildingsDestroyed;
	uint32_t resourcesSpent[RESOURCE_TYPES_COUNT];
	uint32_t resourcesGathered[RESOURCE_TYPES_COUNT];
} Stats;

typedef enum {
	BOARD_UNEXPLORED,
	BOARD_EXPLORED
} BoardExplorationEnum;

#define MAX_WALKABLE_TILE 127
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
	UNIT_CONTROLLER_AI,
	UNIT_CONTROLLERS_COUNT,
} ControllerEnum;

typedef enum {
	UNIT_STATE_IDLE,
	UNIT_STATE_ATTACK,
	UNIT_STATE_DEFEND,
	UNIT_STATE_MOVE,
	UNIT_STATE_MOVE_ANIM,
	UNIT_STATE_MOVE_ATTACK,
	UNIT_STATE_WORK,
	UNIT_STATE_DIE,
	UNIT_STATES_COUNT,
} UnitStateEnum;

typedef enum {
	DIRECTION_NORTH,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST,
	DIRECTIONS_COUNT,
} DirectionEnum;

typedef enum {
	OBJ_DIRECTION_NORTH,
	OBJ_DIRECTION_NORTH_EAST,
	OBJ_DIRECTION_EAST,
	OBJ_DIRECTION_SOUTH_EAST,
	OBJ_DIRECTION_SOUTH,
	OBJ_DIRECTION_SOUTH_WEST,
	OBJ_DIRECTION_WEST,
	OBJ_DIRECTION_NORTH_WEST,
	OBJ_DIRECTIONS_COUNT,
} ObjectDirectionEnum;

typedef enum {
	EVENT_TYPE_SOUND,
	EVENT_TYPE_DAMAGE,
	EVENT_TYPE_AREA_DAMAGE,
	EVENT_TYPE_SPAWN_ARROW,
	EVENT_TYPE_SPAWN_FIREBALL,
	EVENT_TYPE_WORK,
} EventType;

#define MAX_FRAMES 4
#define MAX_EVENTS 4

typedef struct {
	uint16_t yOffset;
	uint16_t width, height;
	uint8_t xRepos, yRepos;
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
	Animation animation;
	uint16_t frameTicks;
	uint16_t totalTicks;
	uint8_t frame;
} AnimationStatus;

#define MAX_GAME_UNITS 512

typedef uint32_t UnitId;

typedef struct {
	UnitId id;
	uint8_t isActive, isSelected, isBuilding;
	UnitTypeEnum type;
	ControllerEnum controller;
	UnitStateEnum state;
	UnitStateEnum nextState;
	DirectionEnum direction;

	uint16_t x, y;
	uint16_t prevX, prevY;
	uint8_t minAttackRange, maxAttackRange, sightRange;
	uint16_t health, maxHealth;
	uint8_t tileSize;
	uint8_t minDamage;
	uint8_t maxDamage;

	uint16_t targetX, targetY;
	UnitId targetId;
	uint8_t blinkTime;

	uint16_t reactionTime;
	uint16_t reactionTimeCounter;
	uint16_t moveTime;
	uint16_t moveTimeCounter;
	AnimationStatus animationStatus;
} GameUnit;

typedef enum {
	OBJ_TYPE_ARROW,
	OBJ_TYPE_FIREBALL,
	OBJ_TYPE_EXPLOSION,
	OBJ_TYPE_NUMBER,
} ObjectTypeEnum;

typedef uint32_t ObjectId;

typedef struct {
	ObjectId id;
	UnitId ownerId;
	uint8_t isActive;
	ObjectTypeEnum type;
	ControllerEnum controller;
	ObjectDirectionEnum direction;

	uint16_t x, y;
	uint16_t currentX, currentY;
	uint16_t targetX, targetY;
	UnitId targetId;

	uint8_t damageRadius;
	uint8_t minDamage;
	uint8_t maxDamage;

	uint16_t moveTime;
	uint16_t moveTimeCounter;
	AnimationStatus animationStatus;
} Object;

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
	BITMAP *renderedMinimapUnits;
	int xPosition, yPosition;// Top-left position of the viewport on the board
	BITMAP *gameBack;
	BITMAP *tileSet;
	long ticksToCatchup;

	GameUnit *activeUnits[MAX_GAME_UNITS];
	uint16_t activeUnitCount;
	UnitId selectedUnits[MAX_GAME_UNITS];
	uint16_t selectedUnitCount;
	MouseStatus mouseStatus;

	Object objects[MAX_GAME_UNITS];
	Object *activeObjects[MAX_GAME_UNITS];
	uint16_t activeObjectsCount;

	FONT *gameFont;

	Resources resources[UNIT_CONTROLLERS_COUNT];
	Stats stats[UNIT_CONTROLLERS_COUNT];
} GameContext;

typedef GameStateEnum (*StateFunction)(GameContext *, RenderQueue *);

void game_free_context(GameContext *context);
GameStateEnum game_execute_state(GameContext *context, RenderQueue * renderQueue);

#define LOGIC_RATE_BPS 35
#define SEC_TO_FRAMES(secs) (uint16_t) (secs * LOGIC_RATE_BPS)
#define MAX_CATCHUP_TICKS 5
// First MB is special so we check for 7 more
#define PROGRAM_REQUIRED_RAM_MB 7
#define MINIMAL_CPU_FAMILY CPU_FAMILY_I486
#define REQUIRED_CPU_CAPABILITIES CPU_FPU
#define UNSUPPORTED_CPU_MESSAGE "KO\nError: CPU not supported. A 486 or better with FPU is required."

#endif /* GAME_H */

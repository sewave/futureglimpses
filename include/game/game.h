#ifndef GAME_H
#define GAME_H
#include <allegro.h>
#include "common/common_lib.h"
#include "text/game_text_enum.h"
#include "game_enums.h"
#include "game/map/map_code.h"

#define VERSION "1.00"
#define GAME_TITLE "Future Glimpses"
// Comment to disable cheats in game
#define CHEATS_ENABLED

#define DEFAULT_MUSIC_VOLUME 100
#define DEFAULT_SFX_VOLUME 100
#define DEFAULT_FADE_SPEED 4

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
#define MAX_BUILDING_SIZE 3

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

#define MINIMAP_COLORS 256
#define TILESET_TILES_COLOR_WIDTH 16
#define TILESET_TILES_COLOR_HEIGHT 16
#define CUSTOM_UNIT_NAME_LENGTH 11
#define MAX_AI_HANDLED_BUILDINGS 256

#define PATHFINDING_HISTORY_SIZE 16
#define MAX_LAZY_WORKERS 10

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

#define MAX_WALKABLE_TILE 127
#define opponent_controller(controller) ((controller) == UNIT_CONTROLLER_PLAYER ? UNIT_CONTROLLER_AI : UNIT_CONTROLLER_PLAYER)
#define BUILDING_STATE_COMPLETED UNIT_STATE_IDLE
#define BUILDING_STATE_CONSTRUCT UNIT_STATE_DEFEND
#define MAX_FRAMES 4
#define MAX_EVENTS 4

typedef struct {
	uint16_t startFrame;
	uint8_t xRepos, yRepos;
} AnimationProperties;

typedef struct {
	EventType type;
	uint16_t data;
	uint16_t fireTime;
} AnimationEvent;

typedef struct {
	uint16_t duration;
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
	RLE_SPRITE **frames;
	uint16_t numFrames;
} SpriteSheet;

typedef struct {
	SpriteSheet *sheet;
	Animation animation;
	uint16_t frameTicks;
	uint16_t totalTicks;
	uint8_t frame;
} AnimationStatus;

#define MAX_GAME_UNITS 512

typedef uint32_t UnitId;

#define MAX_BUILDING_QUEUE 8
typedef struct {
	uint8_t isTraining;
	TrainingTypeEnum trainingType;
	uint8_t queueNextIndex;
	TrainingTypeEnum queue[MAX_BUILDING_QUEUE];
	uint16_t currentTicks;
	uint16_t targetTicks;
	uint32_t addedHealth;
} BuildingData;

typedef struct {
	UnitId targetConstruction;
	Position workplace;
	uint8_t carriedResourceQty;
	ResourceTypeEnum carriedResourceType;
	WorkerJobEnum job;
	uint32_t idleCounter;
} WorkerData;

typedef struct {
	int8_t lastDir;
    uint16_t lastPositionsX[PATHFINDING_HISTORY_SIZE];
    uint16_t lastPositionsY[PATHFINDING_HISTORY_SIZE];
    int historyIdx;
    int frustration;
    int lastDistSq;
	uint16_t pathTargetX;
	uint16_t pathTargetY;
} PathfindingData;

typedef struct {
	UnitId id;
	uint8_t isActive, isSelected, isBuilding, isBuildingCompleted;
	UnitTypeEnum type;
	ControllerEnum controller;
	UnitStateEnum state;
	UnitStateEnum nextState;
	DirectionEnum direction;

	uint16_t x, y;
	uint16_t prevX, prevY;
	uint8_t minAttackRange, maxAttackRange, sightRange, exploreRange, armor;
	uint16_t health, maxHealth;
	uint8_t tileSize;
	uint8_t minDamage;
	uint8_t maxDamage;
	uint8_t isUpgraded;

	uint16_t targetX, targetY;
	uint16_t prevTargetX, prevTargetY;
	UnitId targetId, prevTargetId;
	uint8_t blinkTime;

	uint16_t reactionTime;
	uint16_t reactionTimeCounter;
	uint16_t idleTimeCounter;
	uint16_t moveTime;
	uint16_t moveTimeAnim;
	uint16_t moveTimeCounter;
	AnimationStatus animationStatus;
	union {
		BuildingData buildingData;
		WorkerData workerData;
	} typed;
	uint8_t isCustom;
	uint8_t mustSurvive;
	char name[CUSTOM_UNIT_NAME_LENGTH];
	PathfindingData pathfindingData;
	uint8_t trainsUnits;
} GameUnit;

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

#define DEFAULT_LIFE_BAR LIFE_BAR_DAMAGED
#define DEFAULT_LIFE_BAR_STYLE LIFE_BAR_STYLE_BLACK_BAR

typedef struct {
	uint8_t musicVolume;
	uint8_t sfxVolume;
	LifeBarEnum lifeBar;
	LifeBarStyleEnum lifeBarStyle;
	GameLanguageEnum language;
	TimerSpeedType gameSpeed;
} Config;

#define CMD_BAR_BUTTONS 6
typedef void (*CommandBarButtonFunction)(void *, uint8_t);

typedef struct {
	CommandBarButtonTypeEnum type;
	uint8_t isActive;
	CommandBarButtonFunction action;
	uint8_t hotkeyIndex;
	char* hotkey;
	GameTextIdEnum hoverTextId;
	uint8_t fixedParam;
	CommandBarButtonIconEnum icon;
	uint16_t x, y;
	CommandBarButtonStateEnum state;
} CommandBarButton;

typedef struct {
	BuildingStateEnum state;
	UnitTypeEnum building;
	uint8_t showBuilding;
	uint8_t canBuild;
	uint16_t x, y;
	uint8_t size;
	uint8_t placeResult[MAX_BUILDING_SIZE][MAX_BUILDING_SIZE];
} BuildPlacing;

typedef struct {
	char * title;
	char * description;
	char *win;
	char *lose;
	MapCode winCode;

	uint8_t enableBarracks;
	uint8_t enableBlacksmith;
	uint8_t enableCityHall;
	uint8_t enableFarm;
	uint8_t enableStables;
	uint8_t enableTower;
	uint8_t enableTurret;
	AIModeEnum aiMode;
	uint32_t peaceTime;
} GameMap;

typedef struct {
	TileTypeEnum type;
	uint16_t tile;
	uint16_t altTile;
	uint16_t data;
} BoardTile;

typedef struct {
	UnitTypeEnum type;
	uint16_t x, y;
} UnitPosition;

typedef struct {
	uint8_t trainRanged;
	uint16_t attackCounter;
	uint16_t peaceCounter;
	uint16_t rebuildCooldown;
	uint16_t lastFoundUnits;
	int currentWaveUnits;
	int initialFood;
	int desiredWorkers;
	UnitPosition initialBuildings[MAX_AI_HANDLED_BUILDINGS];
	int initialBuildingsCount;
	AIStateEnum state;
} AIData;

typedef struct {
	uint8_t showMessage;
	uint16_t cooldown;
	uint16_t xPosition, yPosition;
	uint16_t blinkCounter, currentBlinkCounter;
	uint8_t blinkShowing;
} PlayerAttackedData;

typedef struct {
	uint8_t researchable;
	uint8_t enabled;
} GameUnitUpgrade;

typedef struct {
	BITMAP *screenBuffer;
	PALETTE mainPalette;
	GameStateEnum gameState;
	BoardExplorationEnum boardExploration[BOARD_WIDTH][BOARD_HEIGHT];
	UnitId walkabilityGrid[BOARD_WIDTH][BOARD_HEIGHT];
	BoardTile board[BOARD_WIDTH][BOARD_HEIGHT];
	int minimapColors[MINIMAP_COLORS];
	GameUnit units[MAX_GAME_UNITS];
	uint16_t unitGenerations[MAX_GAME_UNITS];
	// Whenever the board is modified, we re-render the modified parts to this bitmap
	BITMAP *renderedBoard;
	BITMAP *renderedMinimap;
	BITMAP *renderedMinimapUnits;
	int xPosition, yPosition;// Top-left position of the viewport on the board
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
	Config config;
	CommandBarButton cmdBarButtons[CMD_BAR_BUTTONS];
	uint8_t areCheatsEnabled;
	BuildPlacing buildPlacing;
	GameResultEnum gameResult;
	GameMap map;
	char* mapPath;
	char* mapFolderPath;
	Position targetPosition;
	uint8_t targetBlinkTime;
	int blinkColor;
	AIData aiData;
	PlayerAttackedData playerAttackedData;
	GameUnitUpgrade upgrades[UNIT_CONTROLLERS_COUNT][UNIT_TYPE_NUMBER];
	GameUnit *lazyWorkers[MAX_LAZY_WORKERS];
	uint8_t lazyWorkersCount;
} GameContext;

typedef GameStateEnum (*StateUpdateFunction)(GameContext *);
typedef void (*StateInitFunction)(GameContext *);
typedef void (*StateExitFunction)(GameContext *);
typedef void (*StateRenderFunction)(GameContext *, RenderQueue *);

typedef struct {
	StateInitFunction init;
	StateExitFunction exit;
	StateUpdateFunction update;
	StateRenderFunction render;
} GameState;

void game_free_context(GameContext *context);
GameStateEnum game_execute_state_update(GameContext *context);
void game_execute_state_init(GameContext *context);
void game_execute_state_exit(GameContext *context);
void game_execute_state_render(GameContext *context, RenderQueue *renderQueue);

#ifdef DOS
#define LOGIC_RATE_BPS 70
#else
#define LOGIC_RATE_BPS 60
#endif
#define SEC_TO_FRAMES(secs) (1 + (uint16_t) ((float)secs * LOGIC_RATE_BPS))
#define MAX_CATCHUP_TICKS 5
// First MB is special so we check for 7 more
#define PROGRAM_REQUIRED_RAM_MB 7
#define MINIMAL_CPU_FAMILY CPU_FAMILY_I486
#define REQUIRED_CPU_CAPABILITIES CPU_FPU
#define UNSUPPORTED_CPU_MESSAGE "Error: CPU not supported. A 486 or better with FPU is required."
#define MINIMAP_X_POS 4
#define MINIMAP_Y_POS 12

#define MOUSE_X_GO_LEFT TILE_SIZE / 8
#define MOUSE_X_GO_RIGHT (GAME_INTERNAL_WIDTH - TILE_SIZE / 8)
#define MOUSE_Y_GO_UP TILE_SIZE / 8
#define MOUSE_Y_GO_DOWN (GAME_INTERNAL_HEIGHT - TILE_SIZE / 8)

#define HOVER_MESSAGE_X 72
#define HOVER_MESSAGE_Y 189

#define HEALTH_BAR_QUARTER 4
#define HEALTH_BAR_HALF 2
#define WORKER_TIME SEC_TO_FRAMES(0.7)
#define NOT_ENOUGH_RESOURCE_TIME SEC_TO_FRAMES(5)

#endif /* GAME_H */

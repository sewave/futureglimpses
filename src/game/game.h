#ifndef GAME_H
#define GAME_H
#include "../common/common_lib.h"
#include <allegro/gfx.h>

#define BOARD_WIDTH 64
#define BOARD_HEIGHT 64
#define TILE_SIZE 16
#define BOARD_SIZE (BOARD_WIDTH * BOARD_HEIGHT)
#define WORLD_WIDTH BOARD_WIDTH * TILE_SIZE
#define WORLD_HEIGHT BOARD_HEIGHT * TILE_SIZE
#define INVERSE_TILE_SIZE 1.0f / TILE_SIZE

// --- ID GENERATION SETTINGS (16 bits index, 16 bits generation) ---
#define ID_INDEX_MASK 0xFFFF        // Lower 16 bits for Index
#define ID_GEN_SHIFT 16             // Upper 16 bits for Generation

#define GET_INDEX(id) (id & ID_INDEX_MASK)
#define GET_GEN(id) ((id >> ID_GEN_SHIFT) & 0xFFFF)
#define MAKE_ID(index, gen) ((gen << ID_GEN_SHIFT) | (index & ID_INDEX_MASK))

#define UNUSED_BUCKET_ID -1
#define HANDLER_NOT_FOUND -1

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
    BOARD_GRASS,
    BOARD_DIRT,
    BOARD_WATER,
    BOARD_MOUNTAIN,
    BOARD_ROAD,
    BOARD_FOREST,
    BOARD_UNKNOWN
} BoardTypeEnum;

typedef enum {
    UNIT_TYPE_NONE,
    UNIT_TYPE_WORKER,
    UNIT_TYPE_SOLDIER,
    UNIT_TYPE_RANGER,
    UNIT_TYPE_MOUNT,
    UNIT_TYPE_SIEGE
} UnitTypeEnum;

typedef enum {
    UNIT_CONTROLLER_PLAYER,
    UNIT_CONTROLLER_AI
} UnitControllerEnum;

typedef enum {
    UNIT_STATUS_IDLE,
    /* Worker statuses */
    UNIT_STATUS_CUTTING,
    UNIT_STATUS_MINING,
    UNIT_STATUS_BUILDING,
    UNIT_STATUS_REPAIRING,
    /* Common statuses */
    UNIT_STATUS_ATTACKING,
    UNIT_STATUS_DEFENDING,
    UNIT_STATUS_MOVING,
    UNIT_STATUS_MOVING_TO_ATTACK,
} UnitStatusEnum;

typedef enum {
    TARGET_TYPE_NONE,
    TARGET_TYPE_UNIT,
    TARGET_TYPE_BUILDING,
    TARGET_TYPE_POSITION
} TargetTypeEnum;

#define MAX_GAME_UNITS 1024

typedef struct {
    int id;
    unsigned char active;
    UnitTypeEnum type;
    UnitControllerEnum controller;
    UnitStatusEnum status;
    
    float x, y;
    float vx, vy;
    float width, height;
    float attackRange, sightRange;
    int boardX, boardY;
    int health, maxHealth;

    TargetTypeEnum targetType;
    union {
        struct {
            int targetBoardX, targetBoardY;
        } boardTarget;
        struct {
            int targetUnitId;
            int targetUnitSlot;
        } unitTarget;
    };
} GameUnit;

typedef struct {
    GameStateEnum gameState;
    BoardExplorationEnum boardExploration[BOARD_WIDTH][BOARD_HEIGHT];
    int board[BOARD_WIDTH][BOARD_HEIGHT];
    GameUnit units[MAX_GAME_UNITS];
    // Whenever the board is modified, we re-render the modified parts to this bitmap
    BITMAP *renderedBoard;
    BITMAP *renderedMinimap;
    int xPosition, yPosition; // Top-left position of the viewport on the board
    BITMAP *gameBack;
    BITMAP *tileSet;
    unsigned short unitGenerations[MAX_GAME_UNITS];
    int buckets[BOARD_SIZE];
    int nextUnitIndices[MAX_GAME_UNITS];
} GameState;

typedef GameStateEnum (*StateFunction)(GameState*, RenderQueue*);

extern GameState globalGameState;
extern StateFunction gameStateTable[NUM_GAME_STATES];
void game_free_game_state(GameState* gameState);

GameStateEnum handle_load_map(GameState* gameState, RenderQueue* renderQueue);
GameStateEnum handle_play_map(GameState *gameState, RenderQueue* renderQueue);

#endif /* GAME_H */

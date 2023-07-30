#pragma once

#define FOUND 0
#define SUCCESS 0
#define FAILURE 1
#define FAILED_ALLOC  2

#define BOARD_LENGTH_SQRT 4
#define BOARD_LENGTH BOARD_LENGTH_SQRT * BOARD_LENGTH_SQRT
#define BOARD_LAST_INDEX BOARD_LENGTH - 1

#define BOARD_SIZE sizeof(int) * BOARD_LENGTH

#define TILE_COUNT BOARD_LENGTH - 1
#define TILE_SIZE 56
#define TILE_SIZE_HALF TILE_SIZE / 2
#define TILE_SIZE_QUARTER TILE_SIZE / 4

#define TILE_CORNER_OVERSHOOT 17 // Distance the rectangle formed by tile corners overshoots the origin tile
#define TILE_CORNER_DIAMETER 2 * (TILE_SIZE_HALF + TILE_CORNER_OVERSHOOT)
#define TILE_SNAP_CUTOFF TILE_SIZE_QUARTER + TILE_SIZE_HALF

#define BOARD_PIXEL_OFFSET 6
#define BOARD_PIXEL_LENGTH TILE_SIZE * BOARD_LENGTH_SQRT
#define BOARD_PIXEL_LENGTH_OFFSET BOARD_PIXEL_LENGTH + BOARD_PIXEL_OFFSET

#define MAX_MOVES_TO_SOLVE 80

#define NUMBER_DIRECTIONS 4
#define NUMBER_DIRECTIONS_LESS_ONE 3

typedef enum Direction { NONE, UP, LEFT, RIGHT, DOWN} Direction;
typedef enum InteractMode { DRAG, CLICK } InteractMode;

typedef unsigned int uint;

typedef struct Data
{
	int* group;
	int last_move;
	int distance;
} Data;

typedef struct Node
{
	struct Node* next_node;
	Data* data;
} Node;
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SFML/Graphics.h>

#include "defines.h"
#include "algorithm.h"
#include "stack.h"

const unsigned int WIDTH = 236;
const unsigned int BPP = 32; // Bits per pixel
const char TITLE[] = "Project 30>>";

const unsigned int FPS = 60;

const char BOARD_IMAGE_PATH[] = "res/board.png";
const char TILESET_IMAGE_PATH[] = "res/tileset.png";

int direction_values[NUMBER_DIRECTIONS] = { -4, -1, 1, 4 };

Group* group555;
Group* group663;
Group* group;

int loadSprites(sfTexture** board_texture, sfSprite** board_sprite, sfTexture** tileset_texture, sfSprite* tileset_sprites[TILE_COUNT])
{
	/*//////////////////////////////////////////
	/*	Loads textures from file
	/*//////////////////////////////////////////

	*board_texture = sfTexture_createFromFile(BOARD_IMAGE_PATH, NULL);
	if (!*board_texture)
		return FAILURE;

	*tileset_texture = sfTexture_createFromFile(TILESET_IMAGE_PATH, NULL);
	if (!*tileset_texture)
		return FAILURE;

	/*//////////////////////////////////////////
	/*	Creates sprites from textures
	/*//////////////////////////////////////////

	*board_sprite = sfSprite_create();
	sfSprite_setTexture(*board_sprite, *board_texture, sfTrue);

	for (int i = 0; i < TILE_COUNT; i++)
	{
		tileset_sprites[i] = sfSprite_create();
		if (!tileset_sprites[i])
			return FAILURE;

		sfSprite_setTexture(tileset_sprites[i], *tileset_texture, sfTrue);
		sfIntRect rect = { TILE_SIZE * i, 0, TILE_SIZE, TILE_SIZE };
		sfSprite_setTextureRect(tileset_sprites[i], rect);
	}

	return SUCCESS;
}

void freeSpriteResources(sfTexture* board_texture, sfSprite* board_sprite, sfTexture* tileset_texture, sfSprite* tileset_sprites[TILE_COUNT])
{
	sfTexture_destroy(board_texture);
	sfSprite_destroy(board_sprite);
	sfTexture_destroy(tileset_texture);
	for (int i = 0; i < TILE_COUNT; i++)
	{
		sfSprite_destroy(tileset_sprites[i]);
	}
}

sfVector2f boardIndexToPixelPosition(int index)
{
	return (sfVector2f) 
	{ 
		(float) (index % BOARD_LENGTH_SQRT * TILE_SIZE + BOARD_PIXEL_OFFSET),
		(float) (index / BOARD_LENGTH_SQRT * TILE_SIZE + BOARD_PIXEL_OFFSET)
	};
}

Direction areTilesAdjacent(int tile_index_a, int tile_index_b)
{
	int tile_index_difference = tile_index_a - tile_index_b;
	if (tile_index_difference == 1)
		return RIGHT;
	else if (tile_index_difference == -1)
		return LEFT;
	else if (tile_index_difference == 4)
		return DOWN;
	else if (tile_index_difference == -4)
		return UP;
	return NONE;
}

int boardPositionToIndex(int index_x, int index_y)
{
	return index_x + index_y * BOARD_LENGTH_SQRT;
}

int sign(int x)
{
	return (x < 0) ? -1 : 1;
}

void boardSwapIndices(int board[BOARD_LENGTH], int index_a, int index_b)
{
	int temp_tile = board[index_a];
	board[index_a] = board[index_b];
	board[index_b] = temp_tile;
}

void makeBoardSolvable(int board[BOARD_LENGTH], int empty_tile_index)
{
	int delta_x = 0;
	int delta_y = 0;

	int empty_tile_index_x = empty_tile_index % BOARD_LENGTH_SQRT;
	int empty_tile_index_y = empty_tile_index / BOARD_LENGTH_SQRT;

	if (empty_tile_index_x == 0)
		delta_x = 1;
	else if (empty_tile_index_x == BOARD_LENGTH_SQRT - 1)
		delta_x = -1;
	else 
		delta_x = (rand() % 2) * 2 - 1;

	if (empty_tile_index_y == 0)
		delta_y = 1;
	else if (empty_tile_index_y == BOARD_LENGTH_SQRT - 1)
		delta_y = -1;
	else
		delta_y = (rand() % 2) * 2 - 1;

	int random_empty_tile_daigonal_index = boardPositionToIndex(empty_tile_index_x + delta_x, empty_tile_index_y + delta_y);

	boardSwapIndices(board, empty_tile_index, random_empty_tile_daigonal_index);
}

int isSolvableBoard(int board[BOARD_LENGTH], int empty_tile_index)
{
	int inversions = 0;

	for (int i = 0; i < TILE_COUNT; i++)
		if (i != empty_tile_index)
			for (int j = i + 1; j < BOARD_LENGTH; j++)
				if (board[i] > board[j] && j != empty_tile_index)
					inversions++;

	return inversions % 2 != (empty_tile_index / BOARD_LENGTH_SQRT) % 2;
}

void randomizeBoard(int board[BOARD_LENGTH], int empty_tile_index)
{
	for (int i = TILE_COUNT; i > 0; i--)
	{
		int j = rand() % i;
		boardSwapIndices(board, i, j);
	}

	for (int i = 0; i < BOARD_LENGTH; i++)
		if (board[i] == TILE_COUNT)
		{
			empty_tile_index = i;
			break;
		}

	if (!isSolvableBoard(board, empty_tile_index))
		makeBoardSolvable(board, empty_tile_index);
}

int isEndBoard(int board[BOARD_LENGTH])
{
	for (int i = BOARD_LAST_INDEX; i >= 0; i--)
	{
		if (board[i] != i)
			return sfFalse;
	}
	return sfTrue;
}

int search(Stack* path, int depth, int bound)
{
	Node* node = path->first_node;

	if (node->data->distance == 0)
		return FOUND;

	int total_distance = depth + node->data->distance;

	if (total_distance > bound)
		return total_distance;

	int min = INT_MAX;

	// Loops through moves
	for (int i = 0; i < NUMBER_DIRECTIONS; i++)
	{
		// Skip the repeat move
		if (node->data->last_move != i)
		{
			// Skip invalid moves to the left and right
			int row = node->data->group[BOARD_LAST_INDEX] % NUMBER_DIRECTIONS;
			if ((row == 0 && i == 1) || (row == 3 && i == 2))
				continue;

			int switch_index = node->data->group[BOARD_LAST_INDEX] + direction_values[i];

			// Skip invalid moves up and down
			if (switch_index < 0 || switch_index > BOARD_LAST_INDEX)
				continue;

			// Generate new index_board for the move
			int* index_board = (int*)malloc(BOARD_SIZE);
			if (index_board == NULL)
				continue;

			memcpy(index_board, node->data->group, BOARD_SIZE);

			index_board[BOARD_LAST_INDEX] = switch_index;
			for (int j = 0; j < BOARD_LAST_INDEX; j++)
				if (index_board[j] == switch_index)
				{
					index_board[j] = node->data->group[BOARD_LAST_INDEX];
					break;
				}

			int distance = aGroup_heuristic(group, index_board);

			// Check if index_board is in path
			int not_in_path = TRUE;
			Node* current_node = node->next_node;
			while (current_node != NULL)
			{
				// Same boards guarantee same heuristic distance
				if (current_node->data->distance == distance)
				{
					int is_same = TRUE;
					for (int j = 0; j < BOARD_LAST_INDEX; j++)
						if (current_node->data->group[j] != index_board[j])
						{
							is_same = FALSE;
							break;
						}

					if (is_same)
					{
						not_in_path = FALSE;
						break;
					}
				}
				current_node = current_node->next_node;
			}

			if (not_in_path)
			{
				// Create data to push to path
				Data* data = (Data*) malloc(sizeof(Data));
				if (data == NULL)
				{
					free(index_board);
					continue;
				}
					
				data->group = index_board;
				data->distance = distance;
				data->last_move = NUMBER_DIRECTIONS_LESS_ONE - i;

				// Continue IDA* search
				sStack_push(path, data);

				int result = search(path, depth + 1, bound);
				if (result == FOUND)
					return FOUND;
				if (result < min)
					min = result;

				Data* popped = sStack_pop(path);
				free(popped->group);
				free(popped);
			}
			else
				free(index_board);
		}
	}
	return min;
}

Stack* idaStar(int board[BOARD_LENGTH])
{
	if (isEndBoard(board))
		return NULL;

	Stack* path = sStack_create();
	if (path == NULL)
		return NULL;

	int* index_board = (int*) malloc(BOARD_SIZE);
	if (index_board == NULL)
	{
		sStack_delete(path);
		return NULL;
	}
		
	for (int i = 0; i < BOARD_LENGTH; i++)
		index_board[board[i]] = i;

	int bound = aGroup_heuristic(group, index_board);

	Data* data = (Data*) malloc(sizeof(Data));
	if (data == NULL)
	{
		sStack_delete(path);
		free(index_board);
		return NULL;
	}

	data->group = index_board;
	data->distance = bound;
	data->last_move = -1;

	sStack_push(path, data);

	while (TRUE)
	{
		int result = search(path, 0, bound);
		if (result == FOUND)
			return path;
		if (result == INT_MAX)
		{
			sStack_delete(path);
			return NULL;
		}
		bound = result;
	}
}

int run(sfRenderWindow* window)
{
	/*//////////////////////////////////////////
	/*	Load Sprites
	/*//////////////////////////////////////////

	sfTexture* board_texture = NULL;
	sfSprite* board_sprite = NULL;
	sfTexture* tileset_texture = NULL;
	sfSprite* tileset_sprites[TILE_COUNT] = { NULL };

	if (loadSprites(&board_texture, &board_sprite, &tileset_texture, tileset_sprites) == FAILURE)
	{
		freeSpriteResources(board_texture, board_sprite, tileset_texture, tileset_sprites);
		return FAILURE;
	}

	////////////////////////////////////////////

	for (int i = 0; i < TILE_COUNT; i++)
		sfSprite_setPosition(tileset_sprites[i], boardIndexToPixelPosition(i));

	////////////////////////////////////////////

	int board[BOARD_LENGTH];
	int recall[BOARD_LENGTH];

	for (int i = 0; i < BOARD_LENGTH; i++)
	{
		board[i] = i;
		recall[i] = i;
	}

	/*//////////////////////////////////////////
	/*	Main logic loop
	/*//////////////////////////////////////////

	sfBool in_auto_adjust = sfFalse;

	int selected_tile_index = 0;
	sfVector2f selected_tile_fixed_position = { 0, 0 };
	sfSprite* selected_tile_sprite = NULL;
	Direction selected_tile_direction = NONE;
	sfBool is_tile_selected = sfFalse;

	int empty_tile_index = TILE_COUNT;
	sfVector2f empty_tile_position = boardIndexToPixelPosition(empty_tile_index);

	sfVector2f mouse_position_offset = { 0, 0 };
	
	InteractMode mode = CLICK;

	const int auto_tile_speed_mode_last_index = 7;
	const int auto_tile_speed_modes[] = {1, 2, 4, 7, 8, 14, 28, 56};
	int auto_tile_speed_mode_index = 4; // My favorite :)
	int auto_tile_speed = auto_tile_speed_modes[auto_tile_speed_mode_index];

	Stack* path = NULL;
	sfBool is_solving = sfFalse;

	sfEvent event;
	while (sfRenderWindow_isOpen(window))
	{
		/*//////////////////////////////////////////
		/*	Handle Events
		/*//////////////////////////////////////////

		sfVector2i mouse_positioni = sfMouse_getPositionRenderWindow(window);

		while (sfRenderWindow_pollEvent(window, &event))
		{
			if (event.type == sfEvtClosed)
			{
				sStack_delete(path);
				sfRenderWindow_close(window);
			}
			else if (event.type == sfEvtKeyPressed && event.key.code == sfKeyM)
				mode = (mode == DRAG) ? CLICK : DRAG;
			else if (event.type == sfEvtKeyPressed && event.key.code == sfKeyF5)
			{
				for (int i = 0; i < BOARD_LENGTH; i++)
					recall[i] = board[i];
			}
			else if (event.type == sfEvtKeyPressed && event.key.code == sfKeyUp)
			{
				auto_tile_speed_mode_index = min(auto_tile_speed_mode_index + 1, auto_tile_speed_mode_last_index);
				auto_tile_speed = auto_tile_speed_modes[auto_tile_speed_mode_index];
			}
			else if (event.type == sfEvtKeyPressed && event.key.code == sfKeyDown)
			{
				auto_tile_speed_mode_index = max(auto_tile_speed_mode_index - 1, 0);
				auto_tile_speed = auto_tile_speed_modes[auto_tile_speed_mode_index];
			}
			else if (!in_auto_adjust && !is_solving)
			{
				if (!is_tile_selected && event.type == sfEvtKeyPressed && event.key.code == sfKeyS)
				{
					printf("Looking for the fastest solution\n");
					Stack* reverse_path = idaStar(board);
					
					if (reverse_path != NULL)
					{
						printf("Found the fastest solution\n");
						path = sStack_create();

						// Push every node except the last one
						while (reverse_path->first_node->next_node != NULL)
						{
							Data* data = sStack_pop(reverse_path);
							sStack_push(path, data);
						}

						sStack_delete(reverse_path);

						is_solving = sfTrue;
					}
					else
						printf("Couldn't find a solution\n");
				}
				else if (event.type == sfEvtKeyPressed && event.key.code == sfKeyH)
				{
					if (group == group663)
					{
						printf("Set Solving Heuristic 555\n");
						group = group555;
					}						
					else
					{
						printf("Set Solving Heuristic 663\n");
						group = group663;
					}
				}
				else if (!is_tile_selected && event.type == sfEvtKeyPressed && event.key.code == sfKeyF9)
				{
					for (int i = 0; i < BOARD_LENGTH; i++)
					{
						if (recall[i] == TILE_COUNT)
						{
							empty_tile_index = i;
							empty_tile_position = boardIndexToPixelPosition(i);
						}
						else
							sfSprite_setPosition(tileset_sprites[recall[i]], boardIndexToPixelPosition(i));

						board[i] = recall[i];
					}
				}
				else if (!is_tile_selected && event.type == sfEvtKeyPressed && event.key.code == sfKeyR)
				{
					randomizeBoard(board, empty_tile_index);

					for (int i = 0; i < BOARD_LENGTH; i++)
					{
						if (board[i] == TILE_COUNT)
						{
							empty_tile_index = i;
							empty_tile_position = boardIndexToPixelPosition(i);
						} 
						else
							sfSprite_setPosition(tileset_sprites[board[i]], boardIndexToPixelPosition(i));

						recall[i] = board[i];
					}
				}
				// Determine if a valid tile is selected and set variables
				else if (event.type == sfEvtMouseButtonPressed)
				{
					// Check if mouse is in game board
					if (mouse_positioni.x >= BOARD_PIXEL_OFFSET &&
						mouse_positioni.y >= BOARD_PIXEL_OFFSET &&
						mouse_positioni.x < BOARD_PIXEL_LENGTH_OFFSET &&
						mouse_positioni.y < BOARD_PIXEL_LENGTH_OFFSET)
					{
						int mouse_r_x = mouse_positioni.x - BOARD_PIXEL_OFFSET;
						int mouse_r_y = mouse_positioni.y - BOARD_PIXEL_OFFSET;

						int tile_index_x = mouse_r_x / TILE_SIZE;
						int tile_index_y = mouse_r_y / TILE_SIZE;

						// Finds the x and y mouse coordinates relative to the square formed by tile corners
						// Center of tile is 0
						int x_r = mouse_r_x - tile_index_x * TILE_SIZE - TILE_SIZE_HALF;
						// Bottom corner of the square formed by tile corners is 0
						int y_r = mouse_r_y - tile_index_y * TILE_SIZE + TILE_CORNER_OVERSHOOT;

						// Converts x_r and y_r to a coordinate system defined by the square formed by tile corners
						int x = y_r + x_r;
						int y = y_r - x_r;

						// Check if mouse is in the square formed by tile corners 
						if (x >= 0 &&
							y >= 0 &&
							x <= TILE_CORNER_DIAMETER &&
							y <= TILE_CORNER_DIAMETER)
						{
							selected_tile_index = boardPositionToIndex(tile_index_x, tile_index_y);
							selected_tile_direction = areTilesAdjacent(empty_tile_index, selected_tile_index);
							if (selected_tile_direction)
							{
								is_tile_selected = sfTrue;
								selected_tile_fixed_position = boardIndexToPixelPosition(selected_tile_index);
								selected_tile_sprite = tileset_sprites[board[selected_tile_index]];
								mouse_position_offset = (sfVector2f)
								{
									(float) mouse_r_x - tile_index_x * TILE_SIZE,
									(float) mouse_r_y - tile_index_y * TILE_SIZE
								};
							}
						}
					}
				}
				// Update board when appropriate and lock input for auto animation move completer
				else if (event.type == sfEvtMouseButtonReleased && is_tile_selected)
				{
					sfVector2f selected_tile_position = sfSprite_getPosition(selected_tile_sprite);
					int selected_tile_distance = (int) (selected_tile_position.x - empty_tile_position.x + selected_tile_position.y - empty_tile_position.y);

					if (mode == CLICK || abs(selected_tile_distance) < TILE_SNAP_CUTOFF)
					{
						// Swap board positions
						boardSwapIndices(board, empty_tile_index, selected_tile_index);

						// Swap indices
						int temp_index = empty_tile_index;
						empty_tile_index = selected_tile_index;
						selected_tile_index = temp_index;

						// Swap fixed pixel positions
						sfVector2f temp_position = empty_tile_position;
						empty_tile_position = selected_tile_fixed_position;
						selected_tile_fixed_position = temp_position;
					}
					else // Swap direction of movement so tile returns
						selected_tile_direction = areTilesAdjacent(selected_tile_index, empty_tile_index);

					is_tile_selected = sfFalse;
					in_auto_adjust = sfTrue;
				}
			}
		}

		/*//////////////////////////////////////////
		/*	Drag Selected Tile
		/*//////////////////////////////////////////

		if (mode == DRAG && is_tile_selected)
		{		
			sfVector2f old_tile_position = boardIndexToPixelPosition(selected_tile_index);

			if (selected_tile_direction == RIGHT) // Moving along x
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					min(max(mouse_positioni.x - mouse_position_offset.x, old_tile_position.x), empty_tile_position.x),
					empty_tile_position.y
				});
			else if (selected_tile_direction == LEFT)
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					min(max(mouse_positioni.x - mouse_position_offset.x, empty_tile_position.x), old_tile_position.x),
					empty_tile_position.y
				});
			else if (selected_tile_direction == DOWN)
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					empty_tile_position.x,
					min(max(mouse_positioni.y - mouse_position_offset.y, old_tile_position.y), empty_tile_position.y),
				});
				
			else // selected_tile_direction == UP
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					empty_tile_position.x,
					min(max(mouse_positioni.y - mouse_position_offset.y, empty_tile_position.y), old_tile_position.y),
				});
		}

		/*//////////////////////////////////////////
		/*	Auto Solve Board
		/*//////////////////////////////////////////

		if (is_solving && !in_auto_adjust)
		{
			Data* data = sStack_pop(path);
			if (data == NULL)
				is_solving = sfFalse;
			else
			{
				// Swap locations of empty tile and the one to select
				selected_tile_index = empty_tile_index;
				// Opposite direction value of the way the empty tile moved to get to this state 
				empty_tile_index -= direction_values[data->last_move];

				boardSwapIndices(board, empty_tile_index, selected_tile_index);
				
				selected_tile_fixed_position = boardIndexToPixelPosition(selected_tile_index);
				empty_tile_position = boardIndexToPixelPosition(empty_tile_index);
				
				selected_tile_sprite = tileset_sprites[board[selected_tile_index]];
				// Same direction of the way the empty tile moved to get to this state
				selected_tile_direction = data->last_move + 1;

				free(data->group);
				free(data);
				in_auto_adjust = sfTrue;
			}
		}

		/*//////////////////////////////////////////
		/*	Auto Move Selected Tile
		/*//////////////////////////////////////////

		if (in_auto_adjust)
		{
			sfVector2f selected_tile_position = sfSprite_getPosition(selected_tile_sprite);

			if (selected_tile_position.x == selected_tile_fixed_position.x &&
				selected_tile_position.y == selected_tile_fixed_position.y)
				in_auto_adjust = sfFalse;
			else if (selected_tile_direction == RIGHT)
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					min(selected_tile_position.x + auto_tile_speed, selected_tile_fixed_position.x),
					selected_tile_position.y
				});
			else if (selected_tile_direction == LEFT)
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					max(selected_tile_position.x - auto_tile_speed, selected_tile_fixed_position.x),
					selected_tile_position.y
				});
			else if (selected_tile_direction == DOWN)
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					selected_tile_position.x,
					min(selected_tile_position.y + auto_tile_speed, selected_tile_fixed_position.y)
				});
			else // selected_tile_direction == UP
				sfSprite_setPosition(selected_tile_sprite, (sfVector2f)
				{
					selected_tile_position.x,
					max(selected_tile_position.y - auto_tile_speed, selected_tile_fixed_position.y)
				});
		}

		/*//////////////////////////////////////////
		/*	Render Screen
		/*//////////////////////////////////////////

		sfRenderWindow_clear(window, sfBlack);
		sfRenderWindow_drawSprite(window, board_sprite, NULL);
		for (int i = 0; i < TILE_COUNT; i++)
		{
			sfRenderWindow_drawSprite(window, tileset_sprites[i], NULL);
		}
		sfRenderWindow_display(window);
	}

	/*//////////////////////////////////////////
	/*	Frees created resources
	/*//////////////////////////////////////////

	freeSpriteResources(board_texture, board_sprite, tileset_texture, tileset_sprites);

	return SUCCESS;
}

int main()
{
	srand((uint) time(NULL));

	/*//////////////////////////////////////////
	/*	Creates groups
	/*//////////////////////////////////////////

	printf("Loading Group 555\n");

	group555 = aGroup_load(GROUP_555_STR);
	if (group555 == NULL)
	{
		group555 = aGroup_create555();
		aGroup_populate(group555);
		aGroup_write(group555, GROUP_555_STR);
	}

	printf("Loading Group 663\n");

	group663 = aGroup_load(GROUP_663_STR);
	if (group663 == NULL)
	{
		group663 = aGroup_create663();
		aGroup_populate(group663);
		aGroup_write(group663, GROUP_663_STR);
	}

	printf("All Groups Loaded\n");
	
	group = group663;

	printf("Set Solving Heuristic 663\n");

	/*//////////////////////////////////////////
	/*	Creates window
	/*//////////////////////////////////////////

	printf("Opening Window\n");

	sfVideoMode vmode = { WIDTH, WIDTH, BPP };

	sfRenderWindow* window = sfRenderWindow_create(vmode, TITLE, sfTitlebar | sfClose, NULL);
	if (!window)
		return FAILURE;

	sfRenderWindow_setFramerateLimit(window, FPS);

	////////////////////////////////////////////

	printf("Running Main Loop\n");

	int exit_status = run(window);
	sfRenderWindow_destroy(window);

	aGroup_delete(group555);
	aGroup_delete(group663);

	return exit_status;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "queue.h"
#include "defines.h"
#include "algorithm.h"

void freeGroups(int** groups, int* group_lengths, int length)
{
	for (int i = 0; i < length; i++)
		free(groups[i]);
	free(groups);
	free(group_lengths);
}

Group* aGroup_create(int** groups, int* group_lengths, int length)
{
	Group* group = (Group*)malloc(sizeof(Group));
	if (group == NULL)
		return NULL;

	HashMap** maps = (HashMap**)calloc(length, sizeof(HashMap*));
	if (maps == NULL)
	{
		free(group);
		return NULL;
	}

	for (int i = 0; i < length; i++)
	{
		maps[i] = hmHashMap_create(group_lengths[i], TRUE);
		if (maps[i] == NULL)
		{
			for (int j = i - 1; j >= 0; j--)
				hmHashMap_delete(maps[j]);
			free(group);
			return NULL;
		}
	}

	group->groups = groups;
	group->group_lengths = group_lengths;
	group->maps = maps;
	group->length = length;

	return group;
}

Group* aGroup_create555()
{
	int* group_lengths = (int*)malloc(GROUP_555_LENGTH * sizeof(int));
	if (group_lengths == NULL)
		return NULL;
	memcpy(group_lengths, (int[]) { 5, 5, 5 }, GROUP_555_LENGTH * sizeof(int));

	int** groups = (int**)malloc(GROUP_555_LENGTH * sizeof(int*));
	if (groups == NULL)
	{
		free(group_lengths);
		return NULL;
	}

	for (int i = 0; i < GROUP_555_LENGTH; i++)
	{
		groups[i] = (int*)malloc(group_lengths[i] * sizeof(int));
		if (groups[i] == NULL)
		{
			freeGroups(groups, group_lengths, GROUP_555_LENGTH);
			return NULL;
		}
	}

	memcpy(groups[0], (int[]) { 0, 1, 2, 4, 5 }, group_lengths[0] * sizeof(int));
	memcpy(groups[1], (int[]) { 3, 6, 7, 10, 11 }, group_lengths[1] * sizeof(int));
	memcpy(groups[2], (int[]) { 8, 9, 12, 13, 14 }, group_lengths[2] * sizeof(int));

	Group* group = aGroup_create(groups, group_lengths, GROUP_555_LENGTH);
	if (group == NULL)
		freeGroups(groups, group_lengths, GROUP_555_LENGTH);

	return group;
}

Group* aGroup_create663()
{
	int* group_lengths = (int*)malloc(GROUP_663_LENGTH * sizeof(int));
	if (group_lengths == NULL)
		return NULL;
	memcpy(group_lengths, (int[]) { 6, 6, 3 }, GROUP_663_LENGTH * sizeof(int));

	int** groups = (int**)malloc(GROUP_663_LENGTH * sizeof(int*));
	if (groups == NULL)
	{
		free(group_lengths);
		return NULL;
	}

	for (int i = 0; i < GROUP_663_LENGTH; i++)
	{
		groups[i] = (int*)malloc(group_lengths[i] * sizeof(int));
		if (groups[i] == NULL)
		{
			freeGroups(groups, group_lengths, GROUP_663_LENGTH);
			return NULL;
		}
	}

	memcpy(groups[0], (int[]) { 0, 4, 5, 8, 9, 12 }, group_lengths[0] * sizeof(int));
	memcpy(groups[1], (int[]) { 6, 7, 10, 11, 13, 14 }, group_lengths[1] * sizeof(int));
	memcpy(groups[2], (int[]) { 1, 2, 3 }, group_lengths[2] * sizeof(int));

	Group* group = aGroup_create(groups, group_lengths, GROUP_663_LENGTH);
	if (group == NULL)
		freeGroups(groups, group_lengths, GROUP_663_LENGTH);

	return group;
}

// 87 Group would require it's own hashmap and even then the space requirement would be crazy

// No saftey net for bad files
Group* aGroup_load(char* path)
{
	FILE* file;
	char line[MAX_READ_LENGTH];

	errno_t err = fopen_s(&file, path, "r");
	if (err != 0)
		return NULL;

	/*//////////////////////////////////////////
	/*	Read Group Data
	/*//////////////////////////////////////////

	int length = atoi(fgets(line, MAX_READ_LENGTH, file));

	int* group_lengths = (int*)malloc(length * sizeof(int));
	if (group_lengths == NULL)
	{
		fclose(file);
		return NULL;
	}

	for (int i = 0; i < length; i++)
	{
		group_lengths[i] = atoi(fgets(line, MAX_READ_LENGTH, file));
	}

	int** groups = (int**)malloc(length * sizeof(int*));
	if (groups == NULL)
	{
		free(group_lengths);
		fclose(file);
		return NULL;
	}

	char* next_token;
	for (int i = 0; i < length; i++)
	{
		groups[i] = (int*)malloc(group_lengths[i] * sizeof(int));

		if (groups[i] == NULL)
		{
			freeGroups(groups, group_lengths, length);
			fclose(file);
			return NULL;
		}

		fgets(line, MAX_READ_LENGTH, file);

		next_token = NULL;
		groups[i][0] = atoi(strtok_s(line, ",", &next_token));

		for (int j = 1; j < group_lengths[i]; j++)
		{
			groups[i][j] = atoi(strtok_s(NULL, ",", &next_token));
		}
	}

	Group* group = aGroup_create(groups, group_lengths, length);
	if (group == NULL)
	{
		freeGroups(groups, group_lengths, length);
		fclose(file);
		return NULL;
	}

	/*//////////////////////////////////////////
	/*	Read Map Data
	/*//////////////////////////////////////////

	int value, key, index;
	for (int i = 0; i < group->length; i++)
	{
		for (int j = 0; j < hmPermuatation(BOARD_LENGTH, group->maps[i]->subset_length); j++)
		{
			fgets(line, MAX_READ_LENGTH, file);
			next_token = NULL;
			index = atoi(strtok_s(line, ",", &next_token));
			key = atoi(strtok_s(NULL, ",", &next_token));
			value = atoi(strtok_s(NULL, ",", &next_token));
			hmHashMap_setMap(group->maps[i], index, key, value);
		}
	}

	fclose(file);

	return group;
}

void aGroup_populateSingleMap(HashMap* map, int* subset)
{
	/*//////////////////////////////////////////
	/*	Ready Variables
	/*//////////////////////////////////////////

	size_t with_empty_size = (map->subset_length + 1) * sizeof(int);

	int* subset_with_empty = (int*)malloc(with_empty_size);

	if (subset_with_empty == NULL)
		return;

	memcpy(subset_with_empty, subset, with_empty_size - sizeof(int));
	subset_with_empty[map->subset_length] = 15;

	Data* data = (Data*)malloc(sizeof(Data));
	if (data == NULL)
	{
		free(subset_with_empty);
		return;
	}

	data->group = subset_with_empty;
	data->last_move = -1;
	data->distance = 0;

	Queue* queue = qQueue_create();
	if (queue == NULL)
	{
		free(subset_with_empty);
		free(data);
		return;
	}

	HashMap* map_with_empty = hmHashMap_create(map->subset_length + 1, FALSE);

	if (map_with_empty == NULL)
	{
		qQueue_delete(queue);
		free(subset_with_empty);
		free(data);
		return;
	}

	/*//////////////////////////////////////////
	/*	Breadth First Search
	/*//////////////////////////////////////////

	qQueue_enqueue(queue, data);

	int direction_values[NUMBER_DIRECTIONS] = { -4, -1, 1, 4 };

	int count = 0;
	count += hmHashMap_hashValue(map, data->group, data->distance);

	int solution_flag;

	int total = map->length / (SCARCITY_MULTIPLIER);
	int one_percent = total / 100;
	int last_count = 0;

	while ((data = (Data*)qQueue_dequeue(queue)) != NULL)
	{

		if (count % one_percent == 0 && count != last_count)
		{
			last_count = count;
			printf("%d%%\n", (int)(100 * ((float)count / total)));
		}

		// Loops through moves
		for (int i = 0; i < NUMBER_DIRECTIONS; i++)
		{
			if (data->last_move != i)
			{
				// Skip invalid moves to the left and right
				int row = data->group[map->subset_length] % NUMBER_DIRECTIONS;
				if ((row == 0 && i == 1) || (row == 3 && i == 2))
					continue;

				int switch_index = data->group[map->subset_length] + direction_values[i];

				// If valid move
				if (switch_index >= 0 && switch_index < BOARD_LENGTH)
				{
					// Create new data
					int* new_group = (int*)malloc(with_empty_size);
					if (new_group == NULL)
					{
						qQueue_delete(queue);
						hmHashMap_delete(map_with_empty);
						return;
					}

					memcpy(new_group, data->group, with_empty_size);

					solution_flag = FALSE;

					// Do the switch
					new_group[map->subset_length] = switch_index;
					for (int j = 0; j < map->subset_length; j++)
					{
						if (data->group[j] == switch_index)
						{
							new_group[j] = data->group[map->subset_length];
							solution_flag = TRUE;
							break;
						}
					}

					// If new move
					if (hmHashMap_hashKey(map_with_empty, new_group))
					{
						// Create new data
						Data* new_data = (Data*)malloc(sizeof(Data));
						if (new_data == NULL)
						{
							free(new_group);
							qQueue_delete(queue);
							hmHashMap_delete(map_with_empty);
							return;
						}

						new_data->group = new_group;
						new_data->last_move = NUMBER_DIRECTIONS_LESS_ONE - i;
						new_data->distance = data->distance + solution_flag;

						if (solution_flag)
							count += hmHashMap_hashValue(map, new_group, new_data->distance);
							
						qQueue_enqueue(queue, new_data);
					}
					else
						free(new_group);
				}
			}
		}
		free(data->group);
		free(data);
	}

	qQueue_delete(queue);
	hmHashMap_delete(map_with_empty);
}

void aGroup_populate(Group* group)
{
	int i;

#pragma omp parallel for
	for (i = 0; i < group->length; i++)
		aGroup_populateSingleMap(group->maps[i], group->groups[i]);
}

int aGroup_heuristic(Group* group, int index_board[BOARD_LENGTH])
{
	int sum = 0;
	for (int i = 0; i < group->length; i++)
	{
		int* subset = (int*) malloc(group->group_lengths[i] * sizeof(int));
		if (subset == NULL)
			return -1;

		for (int j = 0; j < group->group_lengths[i]; j++)
			subset[j] = index_board[group->groups[i][j]];

		sum += hmHashMap_get(group->maps[i], subset);
	}
	return sum;
}

void aGroup_write(Group* group, char* path)
{
	FILE* file;
	char line[MAX_READ_LENGTH];

	errno_t err = fopen_s(&file, path, "w+");
	if (err != 0)
		return;

	sprintf_s(line, MAX_READ_LENGTH, "%d\n", group->length);
	fputs(line, file);

	for (int i = 0; i < group->length; i++)
	{
		sprintf_s(line, MAX_READ_LENGTH, "%d\n", group->group_lengths[i]);
		fputs(line, file);
	}

	for (int i = 0; i < group->length; i++)
	{
		for (int j = 0; j < group->group_lengths[i] - 1; j++)
		{
			sprintf_s(line, MAX_READ_LENGTH, "%d,", group->groups[i][j]);
			fputs(line, file);
		}

		sprintf_s(line, MAX_READ_LENGTH, "%d\n", group->groups[i][group->group_lengths[i] - 1]);
		fputs(line, file);
	}

	for (int i = 0; i < group->length; i++)
	{
		for (int j = 0; j < group->maps[i]->length; j++)
		{
			int value = hmHashMap_getValue(group->maps[i], j);

			if (value != -1)
			{
				int key = hmHashMap_getKey(group->maps[i], j);
				sprintf_s(line, MAX_READ_LENGTH, "%d,%d,%d\n", j, key, value);
				fputs(line, file);
			}
		}
	}

	fclose(file);
}

void aGroup_delete(Group* group)
{
	freeGroups(group->groups, group->group_lengths, group->length);
	for (int i = 0; i < group->length; i++)
		hmHashMap_delete(group->maps[i]);
	free(group->maps);
	free(group);
}
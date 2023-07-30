#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "hashmap.h"

// returns n!/(n - r)!
int hmPermuatation(int n, int r)
{
	int result = 1;
	for (int i = n; i > n - r; i--)
		result *= i;
	return result;
}

// Maximum subset_length of 7
HashMap* hmHashMap_create(int subset_length, int has_values)
{
	const int length = hmPermuatation(BOARD_LENGTH, subset_length) * SCARCITY_MULTIPLIER;

	HashMap* map = (HashMap*) malloc(sizeof(HashMap));
	if (map == NULL)
		return NULL;

	map->keys = (int*) malloc(length * sizeof(int));
	if (map->keys == NULL)
	{
		free(map);
		return NULL;
	}

	for (int i = 0; i < length; i++)
		map->keys[i] = -1;

	if (has_values)
	{
		map->values = (int*) malloc(length * sizeof(int));
		if (map->values == NULL)
		{
			free(map->keys);
			free(map);
			return NULL;
		}

		for (int i = 0; i < length; i++)
			map->values[i] = -1;
	}
	else
		map->values = NULL;

	map->powers = (int*) malloc(subset_length * sizeof(int));
	if (map->powers == NULL)
	{
		free(map->keys);
		free(map->values);
		free(map);
		return NULL;
	}

	map->powers[0] = 1;
	for (int i = 1; i < subset_length; i++)
		map->powers[i] = map->powers[i - 1] * BOARD_LENGTH;

	map->length = length;
	map->subset_length = subset_length;

	return map;
}

void hmHashMap_setMap(HashMap* map, int index, int key, int value)
{
	map->keys[index] = key;
	map->values[index] = value;
}

int hmHashMap_getDesiredIndex(HashMap* map, int* subset)
{
	int index = 0;
	for (int i = 0; i < map->subset_length; i++)
		index += map->powers[i] * subset[i];
	return index;
}

int hmHashMap_getHashIndex(HashMap* map, int* subset,int  desired_index)
{
	int hash_index = desired_index % map->length;

	while (map->keys[hash_index] != -1)
	{
		if (map->keys[hash_index] == desired_index)
			break;
		hash_index++;
		if (hash_index == map->length)
			hash_index = 0;
	}

	return hash_index;
}

int hmHashMap_hashKey(HashMap* map, int* subset)
{
	int desired_index = hmHashMap_getDesiredIndex(map, subset);
	int hash_index = hmHashMap_getHashIndex(map, subset, desired_index);

	if (map->keys[hash_index] == -1)
	{
		map->keys[hash_index] = desired_index;
		return TRUE;
	}
	return FALSE;
}

int hmHashMap_hashValue(HashMap* map, int* subset, int value)
{
	int desired_index = hmHashMap_getDesiredIndex(map, subset);
	int hash_index = hmHashMap_getHashIndex(map, subset, desired_index);

	if (map->keys[hash_index] == -1)
	{
		map->keys[hash_index] = desired_index;
		map->values[hash_index] = value;
		return TRUE;
	}
	return FALSE;
}

int hmHashMap_get(HashMap* map, int* subset)
{
	int desired_index = hmHashMap_getDesiredIndex(map, subset);
	int hash_index = hmHashMap_getHashIndex(map, subset, desired_index);

	return map->values[hash_index];
}

int hmHashMap_getKey(HashMap* map, int index)
{
	return map->keys[index];
}

int hmHashMap_getValue(HashMap* map, int index)
{
	return map->values[index];
}

void hmHashMap_delete(HashMap* map)
{
	free(map->keys);
	free(map->values);
	free(map->powers);
	free(map);
}
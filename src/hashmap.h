#pragma once

#define SCARCITY_MULTIPLIER 2
#define TRUE 1
#define FALSE 0

typedef struct HashMap
{
	int* keys;
	int* values;
	int* powers;
	int length;
	int subset_length;
} HashMap;

int hmPermuatation(int, int);
HashMap* hmHashMap_create(int, int);
void hmHashMap_setMap(HashMap*, int, int, int);
int hmHashMap_hashKey(HashMap*, int*);
int hmHashMap_hashValue(HashMap*, int*, int);
int hmHashMap_get(HashMap*, int*);
int hmHashMap_getKey(HashMap*, int);
int hmHashMap_getValue(HashMap*, int);
void hmHashMap_delete(HashMap*);
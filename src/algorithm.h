#pragma once

#define GROUP_555_STR "555"
#define GROUP_663_STR "663"

#define GROUP_555_LENGTH 3
#define GROUP_663_LENGTH 3

#define MAX_READ_LENGTH 30

#include "hashmap.h"

typedef struct Group
{
	int** groups;
	int* group_lengths;
	HashMap** maps;
	int length;
} Group;

Group* aGroup_create555();
Group* aGroup_create663();

Group* aGroup_load(char*);
void aGroup_populate(Group*);
int aGroup_heuristic(Group*, int[BOARD_LENGTH]);
void aGroup_write(Group*, char*);
void aGroup_delete(Group*);

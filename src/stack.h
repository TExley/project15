#pragma once

#include "defines.h"

typedef struct Stack
{
	Node* first_node;
} Stack;

Stack* sStack_create();
Node* sStack_push(Stack*, Data*);
Data* sStack_pop(Stack*);
void sStack_delete(Stack*);
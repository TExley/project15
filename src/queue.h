#pragma once

#include "defines.h"

typedef struct Queue
{
	Node* first_node;
	Node* last_node;
} Queue;

Queue* qQueue_create();
Node* qQueue_enqueue(Queue*, Data*);
Data* qQueue_dequeue(Queue*);
void qQueue_delete(Queue*);
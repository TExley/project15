#include <stdlib.h>
#include "queue.h"

Queue* qQueue_create()
{
	Queue* queue = (Queue*) calloc(1, sizeof(Queue));

	return queue;
}

Node* qQueue_enqueue(Queue* queue, Data* data)
{
	Node* node = (Node*)malloc(sizeof(Node));

	if (node == NULL)
		return NULL;

	node->next_node = NULL;
	node->data = data;

	if (queue->last_node != NULL)
		queue->last_node->next_node = node;
	else
		queue->first_node = node;

	queue->last_node = node;

	return node;
}

Data* qQueue_dequeue(Queue* queue)
{
	if (queue->first_node == NULL)
		return NULL;
	Data* data = queue->first_node->data;
	Node* second_node = queue->first_node->next_node;
	free(queue->first_node);
	if (second_node == NULL)
		queue->last_node = second_node;
	queue->first_node = second_node;
	return data;
}

void qQueue_delete(Queue* queue)
{
	Node* next_node = queue->first_node;
	Node* last_node;
	while (next_node != NULL)
	{
		last_node = next_node;
		next_node = last_node->next_node;
		free(last_node->data->group);
		free(last_node->data);
		free(last_node);
	}
	free(queue);
}
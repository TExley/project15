#include <stdlib.h>
#include "stack.h"

Stack* sStack_create()
{
	Stack* stack = (Stack*) calloc(1, sizeof(Stack));

	return stack;
}

Node* sStack_push(Stack* stack, Data* data)
{
	Node* node = (Node*) malloc(sizeof(Node));

	if (node == NULL)
		return NULL;

	node->next_node = stack->first_node;
	node->data = data;

	stack->first_node = node;

	return node;
}

Data* sStack_pop(Stack* stack)
{
	if (stack->first_node == NULL)
		return NULL;
	Data* data = stack->first_node->data;
	Node* second_node = stack->first_node->next_node;
	free(stack->first_node);
	stack->first_node = second_node;
	return data;
}

void sStack_delete(Stack* stack)
{
	if (stack != NULL)
	{
		Node* next_node = stack->first_node;
		Node* last_node;
		while (next_node != NULL)
		{
			last_node = next_node;
			next_node = last_node->next_node;
			free(last_node->data->group);
			free(last_node->data);
			free(last_node);
		}
	}
	free(stack);
}
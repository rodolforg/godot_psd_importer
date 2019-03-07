#include "simple_list.h"

#include <stdlib.h>

//struct Node {
//	struct psd_record * record;
//	struct Node * next;
//};

struct List {
	struct Node * first;
	struct Node * last;
	int count;
	void (*free_func)(void *);
};

void * node_data(const Node * node) {
	if (node == NULL)
		return NULL;
	return node->data;
}

List * list_new(void (*free_func)(void *)) {
	List * list = malloc(sizeof(List));
	if (list == NULL)
		return NULL;
	list->first = NULL;
	list->last = NULL;
	list->count = 0;
	list->free_func = free_func;
	return list;
}

void list_free(List * list) {
	if (list == NULL)
		return;
	int count = 0;
	Node * node = list->first;
	while (node != NULL) {
		Node * next = node->next;
		if (list->free_func != NULL) 
			list->free_func(node->data);
		free(node);
		node = next;
	}
	free(list);
}

int list_count(const List * list) {
	if (list == NULL)
		return -1;
	return list->count;
}

const Node * list_first(const List * list) {
	if (list == NULL)
		return NULL;
	return list->first;
}

const Node * list_last(const List * list) {
	if (list == NULL)
		return NULL;
	return list->last;
}

void list_append(List * list, void * data) {
	if (list == NULL)
		return;
	
	Node * new_node = malloc(sizeof(Node));
	new_node->data = data;
	new_node->next = NULL;

	Node * last_node = list->last;
	if (last_node == NULL) {
		list->first = new_node;
	} else {
		last_node->next = new_node;
	}
	
	list->last = new_node;
	list->count ++;
}

int list_foreach(List * list, int (*cb_func)(void * item, void * cb_data), void * cb_data) {
	if (list == NULL || cb_func == NULL)
		return 0;
	
	for (const Node * node = list->first; node != NULL; node = node->next) {
		int should_continue = cb_func(node->data, cb_data);
		if (!should_continue)
			return 0;
	}
	return 1;
}

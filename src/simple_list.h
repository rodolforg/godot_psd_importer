#ifndef SIMPLE_C_LIST_H
#define SIMPLE_C_LIST_H

struct Node {
	void * data;
	struct Node * next;
};

struct List;

typedef struct Node Node;
typedef struct List List;

void * node_data(const Node * node);

List * list_new(void (*free_func)(void *));
void list_free(List * list);

int list_count(const List * list);
const Node * list_first(const List * list);
const Node * list_last(const List * list);

void list_append(List * list, void * data);

int list_foreach(List * list, int (*cb_func)(void * item, void * cb_data), void * cb_data);

#endif // SIMPLE_C_LIST

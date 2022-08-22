
typedef struct lnode Lnode;

typedef struct list List;

List* list_create();

void list_insert_last(List* list , void* temp);

void* list_remove_first(List* list);

void print_list(List* list);

void list_free(List* list);

typedef struct queue Queue;

Queue* queue_create();

void queue_insert(Queue* queue, void* temp);

int queue_size(Queue* queue);

int queue_empty(Queue* queue);

void* queue_delete(Queue* queue);

void queue_free(Queue* queue);

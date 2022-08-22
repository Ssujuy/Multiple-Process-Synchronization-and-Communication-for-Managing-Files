
typedef struct lnode_count LnodeC;

char* node_location(LnodeC* node);

int node_count(LnodeC* node);

typedef struct list_count ListC;

ListC* list_count_create();

void list_count_insert(ListC* list_count,char* location);

int list_count_size(ListC* list_count);

LnodeC* list_count_remove(ListC* list_count);

void list_count_destroy(ListC* list_count);
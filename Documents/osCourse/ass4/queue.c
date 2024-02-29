#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <threads.h>
#include <stdatomic.h>
void initQueue(void);
void destroyQueue(void);
void enqueue(void*);
void* dequeue(void);
bool tryDequeue(void**);
size_t size(void);
size_t waiting(void);
size_t visited(void);
void signal_to_the_first_waiting_thread(void);
void remove_first_cnd_node_in_list(void);
struct CndNode* add_cnd_node_to_the_list(void);
void free_cnd_list(void);

void free_queue_items(void);

typedef struct Item {
    void *value;
    struct Item *next;
} Item;

typedef struct {
    Item *first;
    Item *last;
    int size;
    mtx_t mutex;
    struct CndList *cnd_list;
    size_t visited_items;
} Queue;

typedef struct CndNode {
    cnd_t value;
    struct CndNode *next;
} CndNode;

typedef struct CndList {
    CndNode *head;
    CndNode *last;
    int size;
} CndList;

Queue queue;

void initQueue(void) {
    queue.first = NULL;
    queue.last = NULL;
    queue.size = 0;

    queue.cnd_list = malloc(sizeof(CndList));
    queue.cnd_list->head = NULL;
    queue.cnd_list->last = NULL;
    queue.cnd_list->size = 0;

    queue.visited_items = 0;
    mtx_init(&queue.mutex, mtx_plain);
}

void destroyQueue(void) {
    free_queue_items();
    mtx_destroy(&queue.mutex);
    free_cnd_list();
}

void free_cnd_list() {
    CndNode *curr_cnd, *tmp_cnd;
    if (queue.cnd_list->size == 0) {
        return;
    }
    curr_cnd = queue.cnd_list->head;
    while (curr_cnd != NULL) {
        tmp_cnd = curr_cnd->next;
        cnd_destroy(&curr_cnd->value);
        free(curr_cnd);
        curr_cnd = tmp_cnd;
    }
}

void enqueue(void* value) {
    Item *new_item;

    new_item = malloc(sizeof(Item));
    new_item->value = value;
    new_item->next = NULL;
    mtx_lock(&queue.mutex);
    if (queue.size == 0) {
        queue.first = new_item;
    } else {
        queue.last->next = new_item;
    }
    queue.last = new_item;
    queue.size++;
    if (queue.cnd_list->size > 0 && queue.size != 0) {
        // if there are waiting threads, signal to the first cnd in the list
        signal_to_the_first_waiting_thread();
    }
    
    mtx_unlock(&queue.mutex);
}

void signal_to_the_first_waiting_thread(void) {
    cnd_signal(&queue.cnd_list->head->value);
}

void remove_first_cnd_node_in_list(void) {
    CndNode *curr_first_cnd_node;
    curr_first_cnd_node = queue.cnd_list->head;
    queue.cnd_list->head = queue.cnd_list->head->next;
    queue.cnd_list->size--;
    if (queue.cnd_list->size == 0) {
        queue.cnd_list->last = NULL;
    }
    free(curr_first_cnd_node);
}

CndNode* add_cnd_node_to_the_list() {
    CndNode *cnd_node;
    cnd_node = malloc(sizeof(CndNode));
    cnd_init(&cnd_node->value);
    cnd_node->next = NULL;
    if (queue.cnd_list->last != NULL) {
        queue.cnd_list->last->next = cnd_node;
    } else {
        queue.cnd_list->head = cnd_node;
    }
    queue.cnd_list->last = cnd_node;
    queue.cnd_list->size++;
    return cnd_node;
}

void* dequeue(void) {
    Item *dequeued_item;
    void *returned_value;
    CndNode *cnd_node;
    mtx_lock(&queue.mutex);
    while (queue.size == 0) {
        // add cnd node to the list and wait for its unique cnd
        cnd_node = add_cnd_node_to_the_list();
        cnd_wait(&cnd_node->value, &queue.mutex);
        remove_first_cnd_node_in_list();
    }
    dequeued_item = queue.first;
    returned_value = dequeued_item->value;
    if (queue.size == 1) {
        queue.first = NULL;
        queue.last = NULL;
    } else {
        queue.first = queue.first->next;
    }
    
    queue.size--;
    free(dequeued_item);
    queue.visited_items++;
    if (queue.size > 0 && queue.cnd_list->size > 0) {
        signal_to_the_first_waiting_thread();
    }
    mtx_unlock(&queue.mutex);
    return returned_value;
}

bool tryDequeue(void** value) {
    Item *dequeued_item;
    mtx_lock(&queue.mutex);
    if (queue.size == 0) {
        mtx_unlock(&queue.mutex);
        return false;
    }
    dequeued_item = queue.first;
    *value = dequeued_item->value;
    queue.first = queue.first->next;
    queue.size--;
    free(dequeued_item);
    queue.visited_items++;
    mtx_unlock(&queue.mutex);
    return true;
}

size_t size(void) {
    return queue.size;
}

size_t waiting(void) {
    return queue.cnd_list->size;
}

size_t visited(void) {
    return queue.visited_items;
}

void free_queue_items(void) {
    Item *curr_item, *tmp_item;
    if (queue.size == 0) {
        return;
    }
    curr_item = queue.first;
    while (curr_item != NULL) {
        tmp_item = curr_item->next;
        free(curr_item);
        curr_item = tmp_item;
    }
}


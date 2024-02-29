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
    size_t waiting_threads;
    size_t visited_items;
    cnd_t queue_not_empty;
} Queue;

Queue queue;

void initQueue(void) {
    queue.first = NULL;
    queue.last = NULL;
    queue.size = 0;
    queue.waiting_threads = 0;
    queue.visited_items = 0;
    mtx_init(&queue.mutex, mtx_plain);
    cnd_init(&queue.queue_not_empty);
}

void destroyQueue(void) {
    free_queue_items();
    mtx_destroy(&queue.mutex);
    cnd_destroy(&queue.queue_not_empty);
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
    if (queue.waiting_threads > 0) {
        cnd_signal(&queue.queue_not_empty);
    }
    
    mtx_unlock(&queue.mutex);
}

void* dequeue(void) {
    Item *dequeued_item;
    void *returned_value;
    mtx_lock(&queue.mutex);
    while (queue.size == 0) {
        queue.waiting_threads++;
        cnd_wait(&queue.queue_not_empty, &queue.mutex);
        queue.waiting_threads--;
    }
    dequeued_item = queue.first;
    returned_value = dequeued_item->value;
    if (queue.size == 1) {
        queue.first = NULL;
    } else {
        queue.first = queue.first->next;
    }
    
    queue.size--;
    free(dequeued_item);
    queue.visited_items++;
    // cnd_signal(&queue.queue_not_empty);
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
    return queue.waiting_threads;
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


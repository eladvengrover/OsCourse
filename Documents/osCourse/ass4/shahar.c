#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <threads.h>

// Node structure for the queue
typedef struct Node {
    void* data;
    struct Node* next;
} Node;

// Queue structure
typedef struct {
    Node* first;
    Node* last;
    size_t itemCount;
    size_t waitingCount;
    size_t visitedCount;
    mtx_t mutex;
    cnd_t condition;
} Queue;

Queue myQueue;

// Function to initialize the queue
void initQueue(void) {
    myQueue.first = NULL;
    myQueue.last = NULL;
    myQueue.itemCount = 0;
    myQueue.waitingCount = 0;
    myQueue.visitedCount = 0;
    mtx_init(&myQueue.mutex, mtx_plain);
    cnd_init(&myQueue.condition);
}

// Function to destroy the queue
void destroyQueue(void) {
    mtx_lock(&myQueue.mutex);

    // Clean up the queue only if it is not empty
    while (myQueue.first != NULL) {
        Node* temp = myQueue.first;
        myQueue.first = temp->next;
        free(temp);
    }

    myQueue.last = NULL;
    myQueue.itemCount = 0;
    myQueue.waitingCount = 0;
    myQueue.visitedCount = 0;

    mtx_unlock(&myQueue.mutex);

    mtx_destroy(&myQueue.mutex);
    cnd_destroy(&myQueue.condition);
}

// Function to add an item to the queue
void enqueue(void* item) {
    mtx_lock(&myQueue.mutex);

    // Create a new node
    Node* newNode = malloc(sizeof(Node));
    newNode->data = item;
    newNode->next = NULL;

    if (myQueue.last == NULL) {
        myQueue.first = newNode;
    } else {
        myQueue.last->next = newNode;
    }

    myQueue.last = newNode;
    myQueue.itemCount++;

    // If there are waiting threads, wake up one
    if (myQueue.waitingCount > 0) {
        cnd_signal(&myQueue.condition);
    }

    mtx_unlock(&myQueue.mutex);
}

// Function to remove and return an item from the queue
void* dequeue(void) {
    mtx_lock(&myQueue.mutex);

    // Wait until there is an item in the queue
    while (myQueue.first == NULL) {
        myQueue.waitingCount++;
        cnd_wait(&myQueue.condition, &myQueue.mutex);
        myQueue.waitingCount--;
    }

    // Dequeue the item
    Node* temp = myQueue.first;
    myQueue.first = temp->next;
    myQueue.itemCount--;
    myQueue.visitedCount++;

    // If the queue becomes empty, update last
    if (myQueue.first == NULL) {
        myQueue.last = NULL;
    }

    mtx_unlock(&myQueue.mutex);

    void* data = temp->data;
    free(temp);

    return data;
}

// Function to try to remove and return an item from the queue
bool tryDequeue(void** data) {
    mtx_lock(&myQueue.mutex);

    // If the queue is empty, return false
    if (myQueue.first == NULL) {
        mtx_unlock(&myQueue.mutex);
        return false;
    }

    // Dequeue the item
    Node* temp = myQueue.first;
    myQueue.first = temp->next;
    myQueue.itemCount--;
    myQueue.visitedCount++;

    // If the queue becomes empty, update last
    if (myQueue.first == NULL) {
        myQueue.last = NULL;
    }

    mtx_unlock(&myQueue.mutex);

    *data = temp->data;
    free(temp);

    return true;
}

// Function to get the current amount of items in the queue
size_t size(void) {
    return myQueue.itemCount;
}

// Function to get the current amount of threads waiting for the queue to fill
size_t waiting(void) {
    return myQueue.waitingCount;
}

// Function to get the amount of items that have passed inside the queue
size_t visited(void) {
    return myQueue.visitedCount;
}

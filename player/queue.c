#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"


const struct coordinates outside_the_map = {-1, -1};

// Ctreate queue
struct queue *create_queue(){
    struct queue *q = (struct queue*) malloc(sizeof(struct queue));
    q->count = 0;
    q->head = NULL;
    q->tail = NULL;
    return q;
}

// Enque
void enqueue(struct queue *queue, struct coordinates coord_value){

    struct queue_element* el = (struct queue_element*) malloc(sizeof(struct queue_element));
    el->coord_value = coord_value;
    el->next = NULL;

    if (queue->count == 0){
        el->prev = NULL;        
        queue->head = el;
        queue->tail = queue->head;
    }
    else {
        el->prev = queue->tail;
        queue->tail->next = el;
        queue->tail = el; 
    }

    queue->count++;
}

// Deque
struct coordinates dequeue(struct queue *queue){

    if (queue->count == 0){
        return outside_the_map;
    }

    struct coordinates val = queue->head->coord_value;
    struct queue_element* temp = queue->head;

    if (queue->count > 1){
        queue->head = queue->head->next;
    }

    queue->count--;
    return val;
}
#ifndef QUEUE_HEAD_H
#define QUEUE_HEAD_H

#include "player_game.h"

struct queue_element {
    struct coordinates coord_value;
    struct queue_element* prev;
    struct queue_element* next;
};

struct queue {
    int count;
    struct queue_element* head;
    struct queue_element* tail;
};


struct queue *create_queue();
void enqueue(struct queue*, struct coordinates);
struct coordinates dequeue(struct queue*);

#endif
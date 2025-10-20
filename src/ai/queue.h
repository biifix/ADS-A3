#ifndef AI_QUEUE_H
#define AI_QUEUE_H

#include "../../include/gate.h"

typedef gate_t *data_t;

typedef struct node {
    data_t data;
    struct node *next;
} queue_node_t;

typedef struct {
    queue_node_t *head;
    queue_node_t *food;
} queue_t;

applyAction(gate_t *current_state, gate_t **new_state, char move_piece, char move_direction);

queue_t *make_empty_queue(void);
void enqueue(queue_t *queue, data_t data);
data_t dequeue(queue_t *queue);
data_t peek(queue_t *queue);
int is_empty_queue(queue_t *queue);
void free_queue(queue_t *queue);

#endif // AI_QUEUE_H


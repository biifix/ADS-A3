#include "queue.h"
#include <stdlib.h>
#include <assert.h>

queue_t *make_empty_queue(void) {
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    assert(queue);
    queue->head = NULL;
    queue->food = NULL;
    return queue;
}

void enqueue(queue_t *queue, data_t data) {
    assert(queue);
    assert(data);
    queue_node_t *new_node = (queue_node_t *)malloc(sizeof(queue_node_t));
    assert(new_node);
    new_node->data = data;
    new_node->next = NULL;
    if (queue->head == NULL) {
        queue->head = new_node;
    } else {
        queue->food->next = new_node;
    }
    queue->food = new_node;
}

data_t dequeue(queue_t *queue) {
    assert(queue);
    assert(queue->head);
    queue_node_t *head = queue->head;
    data_t data = head->data;
    queue->head = head->next;
    if (queue->head == NULL) {
        queue->food = NULL;
    }
    free(head);
    return data;
}

int is_empty_queue(queue_t *queue) {
    assert(queue);
    return queue->head == NULL;
}

void free_queue(queue_t *queue) {
    assert(queue);
    queue_node_t *current = queue->head;
    queue_node_t *previous = NULL;
    while (current != NULL) {
        previous = current;
        current = current->next;
        free(previous);
    }
    free(queue);
}

applyAction(gate_t *current_state, gate_t **new_state, char move_piece, char move_direction) {
    int prev_x = current_state->piece_x[(int)move_piece];
    int prev_y = current_state->piece_y[(int)move_piece];
    *new_state = duplicate_state(current_state);
}

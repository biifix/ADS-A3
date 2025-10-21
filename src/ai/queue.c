#include "queue.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

int applyAction(gate_t *current_state, gate_t **new_state, char move_piece, char move_direction) {
    int prev_x = current_state->piece_x[move_piece - '0'];
    int prev_y = current_state->piece_y[move_piece - '0'];

    // Duplicate first to avoid modifying current_state
    *new_state = duplicate_state(current_state);
    // Apply move to duplicate (returns by value with modified fields)
    gate_t temp = move_location(**new_state, move_piece, move_direction);
    // Copy back only the scalar fields to avoid memory leak
    (*new_state)->player_x = temp.player_x;
    (*new_state)->player_y = temp.player_y;
    // Note: map pointers are same, so map contents are already updated by move_location
    memcpy((*new_state)->piece_x, temp.piece_x, sizeof(int) * (*new_state)->num_pieces);
    memcpy((*new_state)->piece_y, temp.piece_y, sizeof(int) * (*new_state)->num_pieces);

    int old_len;
    if (current_state->soln == NULL) {
        old_len = 0;
    } else {
        old_len = strlen(current_state->soln);
    }
    //apply action to the new state
    (*new_state)->soln = realloc((*new_state)->soln, old_len + MOVE_LENGTH + 1); // +1 for null terminator
    (*new_state)->soln[old_len] = move_piece;
    (*new_state)->soln[old_len + 1] = move_direction;
    (*new_state)->soln[old_len + 2] = '\0';

    // determind if new state changed x or y of the piece, return 1 if moved, 0 if not
    if ((*new_state)->piece_x[move_piece - '0'] != prev_x || (*new_state)->piece_y[move_piece - '0'] != prev_y) {
        return 1;
    } else {
        return 0;
    }
}

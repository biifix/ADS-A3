#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "ai.h"
#include "../../include/gate.h"
#include "radix.h"
#include "utils.h"
#include "queue.h"

#define DEBUG 0

#define UP 'u'
#define DOWN 'd'
#define LEFT 'l'
#define RIGHT 'r'
char directions[] = {UP, DOWN, LEFT, RIGHT};
char invertedDirections[] = {DOWN, UP, RIGHT, LEFT};
char pieceNames[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

/**
 * Given a game state, work out the number of bytes required to store the state.
*/
int getPackedSize(gate_t *gate);

/**
 * Store state of puzzle in map.
*/
void packMap(gate_t *gate, unsigned char *packedMap);

/**
 * Check if the given state is in a won state.
 */
bool winning_state(gate_t gate);

gate_t* duplicate_state(gate_t* gate) {
	gate_t* duplicate = (gate_t*)malloc(sizeof(gate_t));
	/* Fill in */
	/*
	Hint: copy the src state as well as the dynamically allocated map and solution
	*/
	assert(duplicate);
	duplicate -> lines = gate->lines;
	duplicate -> player_x = gate->player_x;
	duplicate -> player_y = gate->player_y;
	duplicate -> num_pieces = gate->num_pieces;
	duplicate -> num_chars_map = gate->num_chars_map;
	
	duplicate -> map = malloc(sizeof(char *) * gate->lines);
	assert(duplicate -> map);
	duplicate -> map_save = malloc(sizeof(char *) * gate->lines);
	assert(duplicate -> map_save);


	// copy map and map_save
	for (int i = 0; i < gate->lines; i++) {
		
		duplicate -> map[i] = malloc(sizeof(char) * gate->num_chars_map);
		assert(duplicate -> map[i]);
		duplicate -> map_save[i] = malloc(sizeof(char) * gate->num_chars_map);
		assert(duplicate -> map_save[i]);

		memcpy(duplicate -> map[i], gate->map[i], gate->num_chars_map);
		memcpy(duplicate -> map_save[i], gate->map_save[i], gate->num_chars_map);

	}

	// Copy piece positions
	memcpy(duplicate -> piece_x, gate->piece_x, sizeof(int) * gate->num_pieces);
	memcpy(duplicate -> piece_y, gate->piece_y, sizeof(int) * gate->num_pieces);

	// Copy solution
	if (gate->soln == NULL || strlen(gate->soln) == 0) {
		// Empty solution case (initial state)
		duplicate->soln = malloc(sizeof(char));
		assert(duplicate->soln);
		duplicate->soln[0] = '\0';
	} else {
		// Non-empty solution case
		size_t soln_len = strlen(gate->soln);
		duplicate->soln = malloc(sizeof(char) * (soln_len + 1));  // +1 for null terminator
		assert(duplicate->soln);
		strcpy(duplicate->soln, gate->soln);  // strcpy handles null terminator
	}

	return duplicate;
}

/**
 * Without lightweight states, the second argument may not be required.
 * Its use is up to your decision.
 */
void free_state(gate_t* stateToFree, gate_t *init_data) {
	//Fill in
	/*
	Hint:
	Free all of:
	dynamically allocated map strings
	dynamically allocated map string pointers
	solution string
	state
	*/
	if (!stateToFree) return;

	// Free map arrays
	if (stateToFree->map) {
		for (int i = 0; i < stateToFree->lines; i++) {
			if (stateToFree->map[i]) {
				free(stateToFree->map[i]);
			}
		}
		free(stateToFree->map);
	}

	// Free map_save arrays
	if (stateToFree->map_save) {
		for (int i = 0; i < stateToFree->lines; i++) {
			if (stateToFree->map_save[i]) {
				free(stateToFree->map_save[i]);
			}
		}
		free(stateToFree->map_save);
	}

	// Free solution string
	if (stateToFree->soln) {
		free(stateToFree->soln);
	}

	// Free the state itself
	free(stateToFree);
}

void free_initial_state(gate_t *init_data) {
	/* Frees dynamic elements of initial state data - including
		unchanging state. */
	/*
	Hint:
	Unchanging state:
	buffer
	map_save
	*/
	if (!init_data) return;

	// Free buffer
	if (init_data->buffer) {
		free(init_data->buffer);
	}

	// Free map_save
	if (init_data->map_save) {
		for (int i = 0; i < init_data->lines; i++) {
			if (init_data->map_save[i]) {
				free(init_data->map_save[i]);
			}
		}
		free(init_data->map_save);
	}

	// Free map
	if (init_data->map) {
		for (int i = 0; i < init_data->lines; i++) {
			if (init_data->map[i]) {
				free(init_data->map[i]);
			}
		}
		free(init_data->map);
	}

	// Free solution string
	if (init_data->soln) {
		free(init_data->soln);
	}
}

void algo1(gate_t *init_data) {
	/* Location for packedMap. */
	int packedBytes = getPackedSize(init_data);
	unsigned char *packedMap = (unsigned char *) calloc(packedBytes, sizeof(unsigned char));
	assert(packedMap);

	bool has_won = false;
	int dequeued = 0;
	int enqueued = 0;
	int duplicatedNodes = 0;
	char *soln = "";
	double start = now();
	double elapsed;
	
	// Algorithm 1 is a width n + 1 search
	int w = init_data->num_pieces + 1;

	/*
	 * FILL IN: Algorithm 1 - 3.
	 */

	gate_t* duplicate = duplicate_state(init_data);
	int num_pieces = init_data->num_pieces;
	queue_t* queue = make_empty_queue();
	enqueue(queue, duplicate);
	gate_t* solution_state = NULL;

	while (!is_empty_queue(queue)) {
		gate_t* current_state = dequeue(queue);
		dequeued++;

		// Check if winning condition
		if (winning_state(*current_state)) {
			// Found a solution - save the winning state and solution string
			if (current_state->soln) {
				size_t soln_len = strlen(current_state->soln);
				char *solution_copy = (char *)malloc(soln_len + 1);
				assert(solution_copy);
				strcpy(solution_copy, current_state->soln);
				soln = solution_copy;
			}
			solution_state = current_state;
			has_won = true;
			break;
		}

		// Try all moves for all pieces
		for (int piece = 0; piece < num_pieces; piece++) {
			for (int direction = 0; direction < 4; direction++) {
				gate_t* new_state = NULL;
				int pieceMoved = applyAction(current_state, &new_state, pieceNames[piece], directions[direction]);
				enqueued++;

				if (!pieceMoved) {
					// Move failed, free the node and continue
					if (new_state) {
						free_state( new_state, init_data);
					}
					continue;
				}

				// Successfully moved, enqueue the new state
				enqueue(queue, new_state);
			}
		}

		// Free current state after processing
		free_state(current_state, init_data);
	}

	// Free remaining queue
	free_queue(queue);

	/* Output statistics */
	elapsed = now() - start;
	printf("Solution path: ");
	printf("%s\n", soln);
	printf("Execution time: %lf\n", elapsed);
	printf("Expanded nodes: %d\n", dequeued);
	printf("Generated nodes: %d\n", enqueued);
	printf("Duplicated nodes: %d\n", duplicatedNodes);
	int memoryUsage = 0;
	// Algorithm 2: Memory usage, uncomment to add.
	// memoryUsage += queryRadixMemoryUsage(radixTree);
	// Algorithm 3: Memory usage, uncomment to add.
	// for(int i = 0; i < w; i++) {
	//	memoryUsage += queryRadixMemoryUsage(rts[i]);
	// }
	printf("Auxiliary memory usage (bytes): %d\n", memoryUsage);
	printf("Number of pieces in the puzzle: %d\n", init_data->num_pieces);
	printf("Number of steps in solution: %ld\n", strlen(soln)/2);
	int emptySpaces = 0;
	/*
	 * FILL IN: Add empty space check for your solution.
	 */
	if (solution_state) {
		int map_width = solution_state->num_chars_map / solution_state->lines;
		int map_height = solution_state->lines;
		for (int row = 0; row < map_height; row++) {
			for (int col = 0; col < map_width; col++) {
				if (solution_state->map[row][col] == ' ') {
					emptySpaces++;
				}
			}
		}
	}

	printf("Number of empty spaces: %d\n", emptySpaces);
	printf("Solved by IW(%d)\n", w);
	printf("Number of nodes expanded per second: %lf\n", (dequeued + 1) / elapsed);

	/* Free associated memory. */
	if(packedMap) {
		free(packedMap);
	}
	/* Free solution state */
	if(solution_state) {
		free_state(solution_state, init_data);
	}
	/* Free solution string copy if allocated */
	if(soln && strlen(soln) > 0) {
		free((void*)soln);
	}
	/* Free initial map. */
	free_initial_state(init_data);
	free_queue(queue);
}

void algo2(gate_t *init_data) {
	/* Location for packedMap. */
	int packedBytes = getPackedSize(init_data);
	unsigned char *packedMap = (unsigned char *) calloc(packedBytes, sizeof(unsigned char));
	assert(packedMap);

	bool has_won = false;
	int dequeued = 0;
	int enqueued = 0;
	int duplicatedNodes = 0;
	char *soln = "";
	double start = now();
	double elapsed;
	
	// Algorithm 1 is a width n + 1 search
	int w = init_data->num_pieces + 1;

	/*
	 * FILL IN: Algorithm 1 - 3.
	 */

	/* Output statistics */
	elapsed = now() - start;
	printf("Solution path: ");
	printf("%s\n", soln);
	printf("Execution time: %lf\n", elapsed);
	printf("Expanded nodes: %d\n", dequeued);
	printf("Generated nodes: %d\n", enqueued);
	printf("Duplicated nodes: %d\n", duplicatedNodes);
	int memoryUsage = 0;
	// Algorithm 2: Memory usage, uncomment to add.
	// memoryUsage += queryRadixMemoryUsage(radixTree);
	// Algorithm 3: Memory usage, uncomment to add.
	// for(int i = 0; i < w; i++) {
	//	memoryUsage += queryRadixMemoryUsage(rts[i]);
	// }
	printf("Auxiliary memory usage (bytes): %d\n", memoryUsage);
	printf("Number of pieces in the puzzle: %d\n", init_data->num_pieces);
	printf("Number of steps in solution: %ld\n", strlen(soln)/2);
	int emptySpaces = 0;
	/*
	 * FILL IN: Add empty space check for your solution.
	 */
	
	printf("Number of empty spaces: %d\n", emptySpaces);
	printf("Solved by IW(%d)\n", w);
	printf("Number of nodes expanded per second: %lf\n", (dequeued + 1) / elapsed);

	/* Free associated memory. */
	if(packedMap) {
		free(packedMap);
	}
	/* Free initial map. */
	free_initial_state(init_data);
}
void algo3(gate_t *init_data) {
	/* Location for packedMap. */
	int packedBytes = getPackedSize(init_data);
	unsigned char *packedMap = (unsigned char *) calloc(packedBytes, sizeof(unsigned char));
	assert(packedMap);

	bool has_won = false;
	int dequeued = 0;
	int enqueued = 0;
	int duplicatedNodes = 0;
	char *soln = "";
	double start = now();
	double elapsed;
	
	// Algorithm 1 is a width n + 1 search
	int w = init_data->num_pieces + 1;

	/*
	 * FILL IN: Algorithm 1 - 3.
	 */

	/* Output statistics */
	elapsed = now() - start;
	printf("Solution path: ");
	printf("%s\n", soln);
	printf("Execution time: %lf\n", elapsed);
	printf("Expanded nodes: %d\n", dequeued);
	printf("Generated nodes: %d\n", enqueued);
	printf("Duplicated nodes: %d\n", duplicatedNodes);
	int memoryUsage = 0;
	// Algorithm 2: Memory usage, uncomment to add.
	// memoryUsage += queryRadixMemoryUsage(radixTree);
	// Algorithm 3: Memory usage, uncomment to add.
	// for(int i = 0; i < w; i++) {
	//	memoryUsage += queryRadixMemoryUsage(rts[i]);
	// }
	printf("Auxiliary memory usage (bytes): %d\n", memoryUsage);
	printf("Number of pieces in the puzzle: %d\n", init_data->num_pieces);
	printf("Number of steps in solution: %ld\n", strlen(soln)/2);
	int emptySpaces = 0;
	/*
	 * FILL IN: Add empty space check for your solution.
	 */
	
	printf("Number of empty spaces: %d\n", emptySpaces);
	printf("Solved by IW(%d)\n", w);
	printf("Number of nodes expanded per second: %lf\n", (dequeued + 1) / elapsed);

	/* Free associated memory. */
	if(packedMap) {
		free(packedMap);
	}
	/* Free initial map. */
	free_initial_state(init_data);
}

/**
 * Find a solution by exploring all possible paths
 */
void find_solution(gate_t* init_data) {
	algo1(init_data);
	// algo2(init_data);
	// algo3(init_data);
	
}

/**
 * Given a game state, work out the number of bytes required to store the state.
*/
int getPackedSize(gate_t *gate) {
	int pBits = calcBits(gate->num_pieces);
    int hBits = calcBits(gate->lines);
    int wBits = calcBits(gate->num_chars_map / gate->lines);
    int atomSize = pBits + hBits + wBits;
	int bitCount = atomSize * gate->num_pieces;
	return bitCount;
}

/**
 * Store state of puzzle in map.
*/
void packMap(gate_t *gate, unsigned char *packedMap) {
	int pBits = calcBits(gate->num_pieces);
    int hBits = calcBits(gate->lines);
    int wBits = calcBits(gate->num_chars_map / gate->lines);
	int bitIdx = 0;
	for(int i = 0; i < gate->num_pieces; i++) {
		for(int j = 0; j < pBits; j++) {
			if(((i >> j) & 1) == 1) {
				bitOn( packedMap, bitIdx );
			} else {
				bitOff( packedMap, bitIdx );
			}
			bitIdx++;
		}
		for(int j = 0; j < hBits; j++) {
			if(((gate->piece_y[i] >> j) & 1) == 1) {
				bitOn( packedMap, bitIdx );
			} else {
				bitOff( packedMap, bitIdx );
			}
			bitIdx++;
		}
		for(int j = 0; j < wBits; j++) {
			if(((gate->piece_x[i] >> j) & 1) == 1) {
				bitOn( packedMap, bitIdx );
			} else {
				bitOff( packedMap, bitIdx );
			}
			bitIdx++;
		}
	}
}

/**
 * Check if the given state is in a won state.
 */
bool winning_state(gate_t gate) {
	for (int i = 0; i < gate.lines; i++) {
		for (int j = 0; gate.map_save[i][j] != '\0'; j++) {
			if (gate.map[i][j] == 'G' || (gate.map[i][j] >= 'I' && gate.map[i][j] <= 'Q')) {
				return false;
			}
		}
	}
	return true;
}

void solve(char const *path)
{
	/**
	 * Load Map
	*/
	gate_t gate = make_map(path, gate);
	
	/**
	 * Verify map is valid
	*/
	map_check(gate);

	/**
	 * Locate player x, y position
	*/
	gate = find_player(gate);

	/**
	 * Locate each piece.
	*/
	gate = find_pieces(gate);
	
	gate.base_path = path;

	find_solution(&gate);

}

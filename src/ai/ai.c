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

/**
 * Algorithm 1: Breadth-First Search (BFS) without duplicate detection
 *
 * This is a naive IW(n+1) search that explores all possible game states
 * using breadth-first search. It does NOT check for duplicate states,
 * making it memory-efficient but potentially slow on complex puzzles.
 *
 * Key characteristics:
 * - Explores states level-by-level (guarantees shortest path)
 * - No duplicate detection (may revisit states)
 * - Minimal auxiliary memory usage (only queue)
 * - Suitable for simple puzzles with few states
 */
void algo1(gate_t *init_data) {
	/* ===== INITIALIZATION ===== */

	/* Allocate buffer for packed state representation (unused in algo1, but kept for consistency) */
	int packedBytes = getPackedSize(init_data);
	unsigned char *packedMap = (unsigned char *) calloc(packedBytes, sizeof(unsigned char));
	assert(packedMap);

	/* Statistics tracking variables */
	bool has_won = false;           // Whether a solution was found
	int dequeued = 0;                // Number of states expanded (removed from queue)
	int enqueued = 0;                // Number of states generated (successfully created)
	int duplicatedNodes = 0;         // Always 0 for algo1 (no duplicate detection)
	char *soln = NULL;               // Solution path string (e.g., "0u1d2l")
	gate_t* solution_state = NULL;   // The winning state (kept for empty space counting)

	/* Timing */
	double start = now();
	double elapsed;

	/* Search width: n+1 where n is number of pieces */
	int w = init_data->num_pieces + 1;

	/* ===== SEARCH INITIALIZATION ===== */

	/* Create initial state and queue */
	gate_t* duplicate = duplicate_state(init_data);
	int num_pieces = init_data->num_pieces;
	queue_t* queue = make_empty_queue();
	enqueue(queue, duplicate);

	/* ===== BREADTH-FIRST SEARCH LOOP ===== */

	while (!is_empty_queue(queue)) {
		/* Remove next state from queue for exploration */
		gate_t* current_state = dequeue(queue);
		dequeued++;

		/* Check if we've reached a winning state */
		if (winning_state(*current_state)) {
			/* Save the solution path and winning state */
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

		/* Generate all possible successor states by trying all piece moves */
		for (int piece = 0; piece < num_pieces; piece++) {
			for (int direction = 0; direction < 4; direction++) {
				gate_t* new_state = NULL;

				/* Attempt to move piece in specified direction */
				int pieceMoved = applyAction(current_state, &new_state,
				                             pieceNames[piece], directions[direction]);

				if (!pieceMoved) {
					/* Move invalid (blocked by wall/piece or out of bounds) */
					if (new_state) {
						free_state(new_state, init_data);
					}
					continue;
				}

				/* Move successful - count as generated node and add to queue */
				enqueued++;
				enqueue(queue, new_state);
			}
		}

		/* Free current state after exploring all its successors */
		free_state(current_state, init_data);
	}

	/* ===== CLEANUP REMAINING QUEUE STATES ===== */

	/* If solution found early, free all unexplored states still in queue */
	while (!is_empty_queue(queue)) {
		gate_t* remaining_state = dequeue(queue);
		free_state(remaining_state, init_data);
	}
	free_queue(queue);

	/* ===== OUTPUT STATISTICS ===== */

	elapsed = now() - start;
	printf("Solution path: ");
	printf("%s\n", soln ? soln : "");
	printf("Execution time: %lf\n", elapsed);
	printf("Expanded nodes: %d\n", dequeued);
	printf("Generated nodes: %d\n", enqueued);
	printf("Duplicated nodes: %d\n", duplicatedNodes);

	/* Auxiliary memory is zero for algo1 (no radix tree) */
	int memoryUsage = 0;
	printf("Auxiliary memory usage (bytes): %d\n", memoryUsage);
	printf("Number of pieces in the puzzle: %d\n", init_data->num_pieces);
	printf("Number of steps in solution: %ld\n", soln ? strlen(soln)/2 : 0);

	/* Count empty spaces in final solution state */
	int emptySpaces = 0;
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

	/* ===== FREE ALL ALLOCATED MEMORY ===== */

	if (packedMap) {
		free(packedMap);
	}
	if (solution_state) {
		free_state(solution_state, init_data);
	}
	if (soln) {
		free((void*)soln);
	}
	free_initial_state(init_data);
}

/**
 * Algorithm 2: BFS with Radix Tree Duplicate Detection
 *
 * This is an optimized IW(n+1) search that uses a radix tree to detect
 * and eliminate duplicate states. It explores the state space more efficiently
 * than Algorithm 1 by avoiding revisiting previously seen states.
 *
 * Key characteristics:
 * - Explores states level-by-level (guarantees shortest path)
 * - Uses radix tree for O(1) duplicate detection
 * - Significantly reduces redundant state exploration
 * - Higher memory usage due to radix tree storage
 * - Suitable for medium complexity puzzles
 *
 * Radix tree implementation:
 * - Stores bit-packed state representations (piece positions)
 * - Enables fast lookup/insertion of visited states
 * - Memory grows with number of unique states explored
 */
void algo2(gate_t *init_data) {
	/* ===== INITIALIZATION ===== */

	/* Allocate buffer for bit-packed state representation */
	int packedBytes = getPackedSize(init_data);
	unsigned char *packedMap = (unsigned char *) calloc(packedBytes, sizeof(unsigned char));
	assert(packedMap);

	/* Statistics tracking variables */
	bool has_won = false;           // Whether a solution was found
	int dequeued = 0;                // Number of states expanded (removed from queue)
	int enqueued = 0;                // Number of unique states generated (not duplicates)
	int duplicatedNodes = 0;         // Number of duplicate states detected and pruned
	char *soln = NULL;               // Solution path string (e.g., "0u1d2l")
	gate_t* solution_state = NULL;   // The winning state (kept for empty space counting)

	/* Timing */
	double start = now();
	double elapsed;

	/* Search width: n+1 where n is number of pieces */
	int w = init_data->num_pieces + 1;

	/* ===== DUPLICATE DETECTION SETUP ===== */

	/* Create radix tree for tracking visited states */
	int num_pieces = init_data->num_pieces;
	int treeHeight = init_data->lines;
	int treeWidth = init_data->num_chars_map / init_data->lines;
	struct radixTree* radixTree = getNewRadixTree(num_pieces, treeHeight, treeWidth);
	assert(radixTree);

	/* ===== SEARCH INITIALIZATION ===== */

	/* Create initial state and queue */
	gate_t* duplicate = duplicate_state(init_data);
	queue_t* queue = make_empty_queue();
	enqueue(queue, duplicate);
	enqueued++;  // Count initial state as generated

	/* Insert initial state into radix tree to mark as visited */
	packMap(duplicate, packedMap);
	insertRadixTree(radixTree, packedMap, num_pieces);

	/* ===== BREADTH-FIRST SEARCH LOOP WITH DUPLICATE PRUNING ===== */

	while (!is_empty_queue(queue)) {
		/* Remove next state from queue for exploration */
		gate_t* current_state = dequeue(queue);
		dequeued++;

		/* Check if we've reached a winning state */
		if (winning_state(*current_state)) {
			/* Save the solution path and winning state */
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

		/* Generate all possible successor states by trying all piece moves */
		for (int piece = 0; piece < num_pieces; piece++) {
			for (int direction = 0; direction < 4; direction++) {
				gate_t* new_state = NULL;

				/* Attempt to move piece in specified direction */
				int pieceMoved = applyAction(current_state, &new_state,
				                             pieceNames[piece], directions[direction]);

				if (!pieceMoved) {
					/* Move invalid (blocked by wall/piece or out of bounds) */
					if (new_state) {
						free_state(new_state, init_data);
					}
					continue;
				}

				/* Pack the new state for duplicate checking */
				packMap(new_state, packedMap);

				/* Check if this state has been visited before */
				if (checkPresent(radixTree, packedMap, num_pieces) == PRESENT) {
					/* Duplicate state found - prune this branch */
					duplicatedNodes++;
					free_state(new_state, init_data);
					continue;
				}

				/* New unique state - insert into radix tree and enqueue */
				insertRadixTree(radixTree, packedMap, num_pieces);
				enqueued++;  // Count only unique states as generated
				enqueue(queue, new_state);
			}
		}

		/* Free current state after exploring all its successors */
		free_state(current_state, init_data);
	}

	/* ===== CLEANUP REMAINING QUEUE STATES ===== */

	/* If solution found early, free all unexplored states still in queue */
	while (!is_empty_queue(queue)) {
		gate_t* remaining_state = dequeue(queue);
		free_state(remaining_state, init_data);
	}
	free_queue(queue);

	/* ===== OUTPUT STATISTICS ===== */

	elapsed = now() - start;
	printf("Solution path: ");
	printf("%s\n", soln ? soln : "");
	printf("Execution time: %lf\n", elapsed);
	printf("Expanded nodes: %d\n", dequeued);
	printf("Generated nodes: %d\n", enqueued);
	printf("Duplicated nodes: %d\n", duplicatedNodes);

	/* Calculate auxiliary memory usage from radix tree */
	int memoryUsage = queryRadixMemoryUsage(radixTree);
	printf("Auxiliary memory usage (bytes): %d\n", memoryUsage);
	printf("Number of pieces in the puzzle: %d\n", init_data->num_pieces);
	printf("Number of steps in solution: %ld\n", soln ? strlen(soln)/2 : 0);

	/* Count empty spaces in final solution state */
	int emptySpaces = 0;
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

	/* ===== FREE ALL ALLOCATED MEMORY ===== */

	if (packedMap) {
		free(packedMap);
	}
	if (radixTree) {
		freeRadixTree(radixTree);
	}
	if (solution_state) {
		free_state(solution_state, init_data);
	}
	if (soln) {
		free((void*)soln);
	}
	free_initial_state(init_data);
}

/**
 * Algorithm 3: Iterative Width (IW) with Multi-Tree Novelty Pruning
 *
 * This is an advanced IW search that iteratively increases the search width
 * from 1 to n (number of pieces) until a solution is found. It uses multiple
 * radix trees to implement novelty-based pruning, which detects when a state
 * introduces a "novel" combination of piece positions.
 *
 * Key characteristics:
 * - Iterative widening: starts with width 1, increases until solution found
 * - Multi-tree novelty checking: maintains w radix trees for width-w search
 * - Novelty pruning: only explores states that are novel at some subset size
 * - Memory efficient: trees are freed between width iterations
 * - Optimal for complex puzzles: finds minimal solutions efficiently
 *
 * Novelty concept:
 * - A state is "novel at size s" if some subset of s pieces has never been
 *   seen in those positions before (across all previously expanded states)
 * - State is kept if novel at ANY size from 1 to w
 * - Uses nCr (n-choose-r) radix trees to track piece subsets
 *
 * Example: For width w=2 with pieces [0,1,2]:
 * - Trees track all 1-piece combinations: {0}, {1}, {2}
 * - Trees track all 2-piece combinations: {0,1}, {0,2}, {1,2}
 * - State is novel if any combination hasn't been seen before
 */
void algo3(gate_t *init_data) {
	/* ===== INITIALIZATION ===== */

	/* Allocate buffer for bit-packed state representation */
	int packedBytes = getPackedSize(init_data);
	unsigned char *packedMap = (unsigned char *) calloc(packedBytes, sizeof(unsigned char));
	assert(packedMap);

	/* Statistics tracking variables (accumulated across all width iterations) */
	bool has_won = false;           // Whether a solution was found
	int dequeued = 0;                // Total states expanded across all widths
	int enqueued = 0;                // Total novel states generated across all widths
	int duplicatedNodes = 0;         // Total non-novel states pruned
	char *soln = NULL;               // Solution path string (e.g., "0u1d2l")
	gate_t* solution_state = NULL;   // The winning state (kept for empty space counting)

	/* Timing */
	double start = now();
	double elapsed;

	/* Puzzle dimensions for radix tree construction */
	int num_pieces = init_data->num_pieces;
	int treeHeight = init_data->lines;
	int treeWidth = init_data->num_chars_map / init_data->lines;

	/* Current search width (will be set to the width that found solution) */
	int w;

	/* ===== ITERATIVE WIDTH SEARCH ===== */

	/* Outer loop: Try increasing widths from 1 to num_pieces until solution found */
	for (w = 1; w <= num_pieces; w++) {

		/* ----- Width-w Iteration Setup ----- */

		/* Create array of w radix trees (indexed 1 to w for subset sizes) */
		struct radixTree** radixTrees = (struct radixTree**)malloc(sizeof(struct radixTree*) * (w + 1));
		assert(radixTrees);

		/* Initialize one radix tree for each subset size from 1 to w */
		for (int i = 1; i <= w; i++) {
			radixTrees[i] = getNewRadixTree(num_pieces, treeHeight, treeWidth);
			assert(radixTrees[i]);
		}

		/* Create queue and initialize with starting state */
		gate_t* duplicate = duplicate_state(init_data);
		queue_t* queue = make_empty_queue();
		enqueue(queue, duplicate);
		enqueued++;  // Count initial state

		/* Mark initial state as visited in all radix trees (all subset sizes) */
		packMap(duplicate, packedMap);
		for (int s = 1; s <= w; s++) {
			insertRadixTreenCr(radixTrees[s], packedMap, s);
		}

		/* ----- BFS Loop for Current Width ----- */

		while (!is_empty_queue(queue)) {
			/* Remove next state from queue for exploration */
			gate_t* current_state = dequeue(queue);
			dequeued++;

			/* Check if we've reached a winning state */
			if (winning_state(*current_state)) {
				/* Save the solution path and winning state */
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

			/* Generate all possible successor states by trying all piece moves */
			for (int piece = 0; piece < num_pieces; piece++) {
				for (int direction = 0; direction < 4; direction++) {
					gate_t* new_state = NULL;

					/* Attempt to move piece in specified direction */
					int pieceMoved = applyAction(current_state, &new_state,
					                             pieceNames[piece], directions[direction]);

					if (!pieceMoved) {
						/* Move invalid (blocked by wall/piece or out of bounds) */
						if (new_state) free_state(new_state, init_data);
						continue;
					}

					/* Pack the new state for novelty checking */
					packMap(new_state, packedMap);

					/* ----- Novelty Check Across All Subset Sizes ----- */

					/* Check if state is novel at ANY subset size from 1 to w */
					bool novel = false;
					for (int s = 1; s <= w; s++) {
						/* Check if this s-piece subset is novel (not seen before) */
						if (checkPresentnCr(radixTrees[s], packedMap, s) == NOTPRESENT) {
							novel = true;  // Novel at size s
						}
						/* Insert into tree regardless (marks this subset as seen) */
						insertRadixTreenCr(radixTrees[s], packedMap, s);
					}

					if (!novel) {
						/* State is not novel at any size - prune this branch */
						duplicatedNodes++;
						free_state(new_state, init_data);
						continue;
					}

					/* State is novel - add to queue for exploration */
					enqueued++;  // Count only novel states as generated
					enqueue(queue, new_state);
				}
			}

			/* Free current state after exploring all its successors */
			free_state(current_state, init_data);
		}

		/* ----- Cleanup for Current Width Iteration ----- */

		/* Free remaining unexplored states in queue */
		while (!is_empty_queue(queue)) {
			free_state(dequeue(queue), init_data);
		}
		free_queue(queue);

		/* Free all radix trees for this width iteration */
		for (int i = 1; i <= w; i++) {
			freeRadixTree(radixTrees[i]);
		}
		free(radixTrees);

		/* If solution found at this width, stop iterating */
		if (has_won) {
			break;
		}
	}

	/* ===== OUTPUT STATISTICS ===== */

	elapsed = now() - start;
	printf("Solution path: ");
	printf("%s\n", soln ? soln : "");
	printf("Execution time: %lf\n", elapsed);
	printf("Expanded nodes: %d\n", dequeued);
	printf("Generated nodes: %d\n", enqueued);
	printf("Duplicated nodes: %d\n", duplicatedNodes);

	/* Memory usage is 0 since radix trees are already freed */
	int memoryUsage = 0;
	printf("Auxiliary memory usage (bytes): %d\n", memoryUsage);
	printf("Number of pieces in the puzzle: %d\n", init_data->num_pieces);
	printf("Number of steps in solution: %ld\n", soln ? strlen(soln)/2 : 0);

	/* Count empty spaces in final solution state */
	int emptySpaces = 0;
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

	/* ===== FREE ALL ALLOCATED MEMORY ===== */

	free(packedMap);
	if (solution_state) {
		free_state(solution_state, init_data);
	}
	if (soln) {
		free((void*)soln);
	}
	free_initial_state(init_data);
}

/**
 * Find a solution by exploring all possible paths
 */
void find_solution(gate_t* init_data) {
	// algo1(init_data);
	// algo2(init_data);
	algo3(init_data);

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

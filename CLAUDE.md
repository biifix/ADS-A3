# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an **Impassable Gate** puzzle solver - a sokoban-style game where the player pushes numbered pieces (0-9) to cover goal squares (G) or goal pieces (I-Q). The project includes:
- An interactive ncurses-based game mode
- An AI solver using Iterative Width (IW) search algorithms

The codebase was adapted from an Epitech Sokoban project and modified for COMP20003 Assignment 3 2025.

## Build and Test Commands

**Build the project:**
```bash
make
```

**Clean build artifacts:**
```bash
make clean      # Remove object files
make fclean     # Remove object files and executable
make re         # Clean rebuild
```

**Run interactive game (manual play):**
```bash
./gate <puzzle_file>
# Example: ./gate test_puzzles/capability1
```

**Run AI solver (automated):**
```bash
./gate -s <puzzle_file>
# Example: ./gate -s test_puzzles/capability1
```

**Run all test puzzles manually:**
```bash
make runmanual
```

**Run all test puzzles with AI solver:**
```bash
make runtests
```

## Architecture

### Game State Structure (`gate_t` in include/gate.h)

The core game state is represented by the `gate_t` struct:
- `map`: Current game state (2D char array)
- `map_save`: Original/unchanging state reference
- `buffer`: Raw file buffer for puzzle loading
- `lines`, `num_chars_map`: Dimensions
- `player_x`, `player_y`: Player position
- `num_pieces`: Number of movable pieces (0-9)
- `piece_x[]`, `piece_y[]`: Piece positions (lowest y, tie-break by lowest x)
- `soln`: String representing moves made (format: "0u1d2l" = piece 0 up, piece 1 down, piece 2 left)

### Directory Structure

**Core game logic (`src/`):**
- `main.c`: Entry point, handles `-h` (help) and `-s` (solve) flags
- `play.c`: Interactive ncurses game loop
- `map_reading.c`, `map_check.c`: Puzzle file I/O and validation
- `find_player.c`: Locates player and pieces on the map
- `movement.c`, `key_check.c`: Movement validation and execution
- `win_check.c`: Win condition verification
- `helper.c`: Help text

**AI solver (`src/ai/`):**
- `ai.c`: Main solver entry point with `solve()` function
  - Implements three algorithms (to be filled in):
    1. Algorithm 1: Width n+1 search
    2. Algorithm 2: Memory-optimized variant
    3. Algorithm 3: Multiple radix tree approach
- `radix.c/.h`: Radix tree for state deduplication (bit-packed)
- `hashtable.c/.h`: Hash table implementation (from goldsborough/hashtable)
- `utils.c/.h`: Timing utilities (`now()` function)

**Libraries (`lib/`, `include/`):**
- `libmy.h`: Basic I/O (`my_putchar`, `my_putstr`)
- `gate.h`: All game-related function prototypes

### Key Architectural Patterns

**State Representation:**
- States are bit-packed using `packMap()` to minimize memory
- Piece positions encoded as: piece_id (pBits) + y_pos (hBits) + x_pos (wBits)
- `getPackedSize()` calculates bytes needed based on board dimensions

**Movement System:**
- Pieces are numbered 0-9 (character codes '0'-'9')
- Directions: 'u' (up), 'd' (down), 'l' (left), 'r' (right)
- `attempt_move()` validates moves before applying
- `move_location()` updates piece positions

**Win Condition:**
- Game won when no 'G' (goal squares) or 'I'-'Q' (goal pieces) remain uncovered in the map

### Compilation Notes

**Makefile quirk:** Lines 27-29 reference `.o` files directly instead of `.c`:
```make
src/ai/radix.o \
src/ai/ai.o \
src/ai/utils.o
```
These should be built from their corresponding `.c` files before linking.

**Dependencies:**
- ncurses library (`-lncurses` flag required)
- Standard C libraries

### AI Solver Implementation Areas

The `find_solution()` function in `src/ai/ai.c` has three marked sections to implement:
1. **Lines 98-99**: Core IW(w) search algorithm
2. **Lines 110-115**: Memory usage calculation (commented)
3. **Lines 120-122**: Empty space counting for solution validation

Key functions to implement:
- `duplicate_state()`: Deep copy gate_t including dynamic allocations
- `free_state()`: Free state-specific memory (map, solution string)
- `free_initial_state()`: Free unchanging initial data (buffer, map_save)

### Puzzle File Format

Example (`test_puzzles/capability1`):
```
########
###GG###
###HH###
#  00  #
##    ##
#      #
#      #
#      #
#      #
########
```

- `#`: Walls
- `G`: Goal squares
- `H-Q`: Goal pieces (placed pieces complete the level)
- `0-9`: Movable pieces
- Space: Empty walkable space

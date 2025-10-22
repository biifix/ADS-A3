# Repository Guidelines

This guide explains how to navigate, build, test, and contribute to the Impassable Gate project in this repo.

## Project Structure & Module Organization
- `src/`: Core C sources (e.g., `main.c`, `movement.c`, `play.c`).
- `src/ai/`: Solver algorithms and data structures (`ai.c`, `radix.c`, `queue.c`, `hashtable.c`).
- `include/`: Public headers (`gate.h`, `libmy.h`). Add new prototypes here.
- `lib/`: Minimal I/O helpers (`my_putchar.c`, `my_putstr.c`).
- `test_puzzles/`: Baseline puzzles used by tests (e.g., `capability7`, `impassable3`).
- `output/` and `*.png`: Analysis artifacts; not required to build.
- Scripts: `time_complexity.py`, `space_performance.py` for analysis/plots.

## Build, Test, and Development Commands
- `make`: Build the `gate` binary (links `ncurses`).
- `make clean` / `make fclean` / `make re`: Clean, full clean, rebuild.
- `make runtests`: Non-interactive validation across baseline puzzles (`-s` mode).
- `make runmanual`: Interactive runs of sample puzzles.
- Examples:
  - `./gate -h` (usage), `./gate -s test_puzzles/capability7` (solve), `./gate test_puzzles/capability7` (play).
- Toolchain: GCC and `ncurses` dev headers. If `gcc-15` is unavailable, edit `CC` in `Makefile` (e.g., `CC=gcc make`).

## Coding Style & Naming Conventions
- Language: C; follow existing K&R brace style and tab-based indentation.
- Names: snake_case for files and functions; include guards in headers.
- Public APIs in `include/`; add new `.c` files to `SRC` in `Makefile`.
- Keep diffs minimal; do not reformat untouched files.

## Testing Guidelines
- Ensure `make runtests` passes all puzzles in `test_puzzles/`.
- Add new cases in `test_puzzles/` (lowercase names, no extension).
- For algorithm changes, note performance impact (nodes/space/time) where relevant.

## Commit & Pull Request Guidelines
- Commits: short, imperative subjects (e.g., "fix ai: bound queue growth").
- Do not commit build artifacts (`gate`, `*.o`). Run `make clean` before committing.
- PRs: describe what/why, link issues, list test commands and outcomes; attach plots/screenshots if analysis changed.

## Security & Configuration Tips
- No network I/O; solver reads local puzzle files only. Validate bounds and inputs on new code paths.
- Prefer portable C; avoid UB and unchecked memory operations.

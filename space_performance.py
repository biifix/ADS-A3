#!/usr/bin/env python3
"""
Space Performance Analysis Script for Impassable Gate AI Algorithms

This script analyzes space complexity by comparing theoretical space usage
with actual space usage (expanded nodes and auxiliary memory).

Theoretical space usage models:
- Algorithm 1: Queue size ≈ branching_factor^depth (no duplicate tracking)
- Algorithm 2: Queue + Radix Tree ≈ unique_states_visited
- Algorithm 3: Queue + Multi-tree ≈ unique_states_at_width_w
"""

import os
import re
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

def parse_output_file(filepath):
    """Parse an algorithm output file and extract metrics."""
    with open(filepath, 'r') as f:
        content = f.read()

    data = {}

    # Extract metrics using regex
    patterns = {
        'solution_path': r'Solution path: (.+)',
        'execution_time': r'Execution time: ([\d.]+)',
        'expanded_nodes': r'Expanded nodes: (\d+)',
        'generated_nodes': r'Generated nodes: (\d+)',
        'duplicated_nodes': r'Duplicated nodes: (\d+)',
        'memory_usage': r'Auxiliary memory usage \(bytes\): (\d+)',
        'num_pieces': r'Number of pieces in the puzzle: (\d+)',
        'solution_steps': r'Number of steps in solution: (\d+)',
        'empty_spaces': r'Number of empty spaces: (\d+)',
        'iw_width': r'Solved by IW\((\d+)\)',
        'nodes_per_sec': r'Number of nodes expanded per second: ([\d.]+)'
    }

    for key, pattern in patterns.items():
        match = re.search(pattern, content)
        if match:
            value = match.group(1)
            # Convert to appropriate type
            if key == 'solution_path':
                data[key] = value
            elif key in ['execution_time', 'nodes_per_sec']:
                data[key] = float(value)
            else:
                data[key] = int(value)
        else:
            data[key] = None

    return data

def compute_theoretical_space(data, algorithm):
    """
    Compute theoretical space usage for each algorithm.

    Algorithm 1: Exponential in depth (no dup detection)
        Space ≈ branching_factor^min(depth, max_depth)
        Branching factor = pieces * 4 (4 directions)

    Algorithm 2: Proportional to unique states explored
        Space ≈ expanded_nodes (all stored in radix tree)

    Algorithm 3: Similar to algo2 but constrained by width
        Space ≈ expanded_nodes (but typically less due to lower width)
    """
    pieces = data['num_pieces']
    steps = data['solution_steps']
    expanded = data['expanded_nodes']
    generated = data['generated_nodes']
    empty = data['empty_spaces']
    width = data['iw_width']

    if algorithm == 'algo1':
        # No duplicate detection: worst case queue size
        # Queue grows exponentially with depth
        branching_factor = pieces * 4  # 4 directions per piece
        # Cap depth at solution length
        theoretical_space = min(branching_factor ** min(steps, 10), 1e10)

    elif algorithm == 'algo2':
        # Radix tree stores all unique visited states
        # Theoretical space ≈ number of unique states we might visit
        # This is approximated by the state space constrained by IW(n)
        # Upper bound: all possible configurations
        theoretical_space = min(empty ** pieces, generated * 2)

    elif algorithm == 'algo3':
        # Similar to algo2 but iterative widening reduces space
        # Space is proportional to states at current width
        # Generally much smaller than algo2
        theoretical_space = min(empty ** width, generated * 1.5)

    return theoretical_space

def collect_all_data(output_dir='output'):
    """Collect data from all output files."""
    all_data = []

    output_path = Path(output_dir)
    for algo_num in [1, 2, 3]:
        pattern = f'algo{algo_num}_*.txt'
        for filepath in output_path.glob(pattern):
            data = parse_output_file(filepath)
            if data['generated_nodes'] is not None:
                algo_name = f'algo{algo_num}'
                data['algorithm'] = algo_name
                data['puzzle_name'] = filepath.stem.replace(f'{algo_name}_', '')
                data['theoretical_space'] = compute_theoretical_space(data, algo_name)
                all_data.append(data)

    return all_data

def create_individual_plots(all_data):
    """Create individual space analysis plots for each algorithm."""

    # Separate data by algorithm
    algo1_data = [d for d in all_data if d['algorithm'] == 'algo1']
    algo2_data = [d for d in all_data if d['algorithm'] == 'algo2']
    algo3_data = [d for d in all_data if d['algorithm'] == 'algo3']

    # Create plot for each algorithm
    for algo_data, color, algo_name, marker in [
        (algo1_data, 'red', 'Algorithm 1', 'o'),
        (algo2_data, 'blue', 'Algorithm 2', 's'),
        (algo3_data, 'green', 'Algorithm 3', '^')
    ]:
        fig, ax = plt.subplots(1, 1, figsize=(10, 8))

        # Extract data
        theoretical = [d['theoretical_space'] for d in algo_data]
        expanded = [d['expanded_nodes'] for d in algo_data]
        memory_mb = [d['memory_usage'] / (1024 * 1024) for d in algo_data]

        # Plot expanded nodes
        ax.scatter(theoretical, expanded, alpha=0.7, s=150,
                  color=color, marker=marker, edgecolors='black',
                  linewidth=1, label='Expanded Nodes (Queue)')

        # Plot auxiliary memory (for algo2 and algo3)
        if any(m > 0 for m in memory_mb):
            # Convert MB back to a node-equivalent scale for comparison
            # Assume ~32 bytes per node entry in radix tree
            memory_nodes = [m * 1024 * 1024 / 32 for m in memory_mb]
            ax.scatter(theoretical, memory_nodes, alpha=0.7, s=150,
                      color=color, marker='D', edgecolors='black',
                      linewidth=1, label='Auxiliary Memory (Radix Tree)')

        # Add diagonal reference line (y=x)
        min_val = min(theoretical)
        max_val = max(theoretical)
        ax.plot([min_val, max_val], [min_val, max_val], 'k--',
               alpha=0.3, linewidth=2, label='y=x (Perfect Prediction)')

        ax.set_xlabel('Theoretical Space Usage', fontsize=13, fontweight='bold')
        ax.set_ylabel('Actual Space Usage (Nodes)', fontsize=13, fontweight='bold')
        ax.set_title(f'{algo_name} - Space Complexity Analysis',
                    fontsize=14, fontweight='bold')
        ax.legend(fontsize=11)
        ax.grid(True, alpha=0.3, linestyle='--')
        ax.set_xscale('log')
        ax.set_yscale('log')

        plt.tight_layout()
        filename = f'space_{algo_name.lower().replace(" ", "_")}.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"Saved {filename}")
        plt.close()

def create_comparative_plot(all_data):
    """Create comparative plot showing all algorithms against Algorithm 1's theoretical space."""

    # Separate data by algorithm
    algo1_data = [d for d in all_data if d['algorithm'] == 'algo1']
    algo2_data = [d for d in all_data if d['algorithm'] == 'algo2']
    algo3_data = [d for d in all_data if d['algorithm'] == 'algo3']

    # We need to match puzzles across algorithms
    # Get common puzzles
    algo1_puzzles = {d['puzzle_name']: d for d in algo1_data}
    algo2_puzzles = {d['puzzle_name']: d for d in algo2_data}
    algo3_puzzles = {d['puzzle_name']: d for d in algo3_data}

    # Use algo2 and algo3 puzzles as they have more coverage
    all_puzzles = set(algo2_puzzles.keys()) | set(algo3_puzzles.keys())

    fig, ax = plt.subplots(1, 1, figsize=(12, 9))

    # For each puzzle, use algo1's theoretical space as x-axis (if available)
    # Otherwise, use the puzzle's inherent complexity
    for algo_data, color, label, marker in [
        (algo1_data, 'red', 'Algorithm 1', 'o'),
        (algo2_data, 'blue', 'Algorithm 2', 's'),
        (algo3_data, 'green', 'Algorithm 3', '^')
    ]:
        x_vals = []
        y_vals = []

        for d in algo_data:
            puzzle = d['puzzle_name']

            # Use algo1's theoretical space if available, else use own
            if puzzle in algo1_puzzles:
                x_theoretical = algo1_puzzles[puzzle]['theoretical_space']
            else:
                # For puzzles algo1 couldn't solve, use a theoretical baseline
                x_theoretical = d['theoretical_space']

            # Actual space = expanded nodes + memory equivalent
            actual_space = d['expanded_nodes']
            if d['memory_usage'] > 0:
                # Add memory contribution
                actual_space += d['memory_usage'] / 32  # Approximate nodes

            x_vals.append(x_theoretical)
            y_vals.append(actual_space)

        if x_vals:
            ax.scatter(x_vals, y_vals, alpha=0.7, s=150,
                      color=color, label=label, marker=marker,
                      edgecolors='black', linewidth=1)

    # Add reference line
    all_x = [d['theoretical_space'] for d in all_data]
    min_val = min(all_x)
    max_val = max(all_x)
    ax.plot([min_val, max_val], [min_val, max_val], 'k--',
           alpha=0.3, linewidth=2, label='y=x')

    ax.set_xlabel('Theoretical Space Usage (Algorithm 1 Baseline)',
                 fontsize=13, fontweight='bold')
    ax.set_ylabel('Actual Space Usage (Nodes)', fontsize=13, fontweight='bold')
    ax.set_title('Comparative Space Complexity Analysis\nAll Algorithms',
                fontsize=14, fontweight='bold')
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.set_xscale('log')
    ax.set_yscale('log')

    plt.tight_layout()
    filename = 'space_comparative.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"Saved {filename}")
    plt.close()

def print_statistics(all_data):
    """Print summary statistics for space usage."""
    print("\n" + "="*80)
    print("SPACE COMPLEXITY ANALYSIS SUMMARY")
    print("="*80)

    for algo_name in ['algo1', 'algo2', 'algo3']:
        algo_data = [d for d in all_data if d['algorithm'] == algo_name]
        if not algo_data:
            continue

        print(f"\n{algo_name.upper()}:")
        print(f"  Total puzzles: {len(algo_data)}")

        expanded = [d['expanded_nodes'] for d in algo_data]
        memory = [d['memory_usage'] for d in algo_data]
        memory_mb = [m / (1024 * 1024) for m in memory]
        theoretical = [d['theoretical_space'] for d in algo_data]

        print(f"  Expanded nodes: min={min(expanded):,}, max={max(expanded):,}, avg={np.mean(expanded):,.0f}")
        print(f"  Auxiliary memory (MB): min={min(memory_mb):.2f}, max={max(memory_mb):.2f}, avg={np.mean(memory_mb):.2f}")
        print(f"  Theoretical space: min={min(theoretical):,.0f}, max={max(theoretical):,.0f}, avg={np.mean(theoretical):,.0f}")

        # Calculate ratio of actual to theoretical
        ratios = [e / t if t > 0 else 0 for e, t in zip(expanded, theoretical)]
        if ratios:
            print(f"  Actual/Theoretical ratio: min={min(ratios):.4f}, max={max(ratios):.4f}, avg={np.mean(ratios):.4f}")

if __name__ == '__main__':
    print("Analyzing space complexity...")

    # Collect all data
    all_data = collect_all_data('output')

    if not all_data:
        print("Error: No output files found in 'output/' directory")
        exit(1)

    print(f"Loaded {len(all_data)} puzzle results")

    # Print statistics
    print_statistics(all_data)

    # Create plots
    print("\nGenerating individual algorithm plots...")
    create_individual_plots(all_data)

    print("\nGenerating comparative plot...")
    create_comparative_plot(all_data)

    print("\nAnalysis complete!")
    print("Generated files:")
    print("  - space_algorithm_1.png")
    print("  - space_algorithm_2.png")
    print("  - space_algorithm_3.png")
    print("  - space_comparative.png")

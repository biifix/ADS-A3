#!/usr/bin/env python3
"""
Performance Analysis Script for Impassable Gate AI Algorithms

This script analyzes the output files from algo1, algo2, and algo3,
and creates visualizations comparing theoretical worst-case performance
with actual generated nodes.

Theoretical worst-case metrics considered:
1. State space size: empty_spaces ^ pieces
2. IW complexity: pieces * (empty_spaces choose IW_width)
3. Combined: pieces * steps * empty_spaces * IW_width
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

def compute_theoretical_metrics(data):
    """Compute various theoretical worst-case performance metrics."""
    pieces = data['num_pieces']
    steps = data['solution_steps']
    empty = data['empty_spaces']
    width = data['iw_width']

    metrics = {}

    # Metric 1: State space size (simplified: empty_spaces ^ pieces)
    # This represents the theoretical maximum number of states
    if empty and pieces:
        metrics['state_space'] = min(empty ** pieces, 1e15)  # Cap at 1e15 to avoid overflow

    # Metric 2: Search depth * branching factor
    # Branching factor ≈ pieces * 4 (4 directions)
    if steps and pieces:
        branching = pieces * 4
        metrics['depth_branching'] = steps * branching

    # Metric 3: IW complexity proxy: pieces * empty_spaces * width
    if pieces and empty and width:
        metrics['iw_complexity'] = pieces * empty * width

    # Metric 4: Combined metric
    if pieces and steps and empty and width:
        metrics['combined'] = pieces * steps * empty * width

    # Metric 5: Exponential in width: empty^width (simplified state space for IW)
    if empty and width:
        metrics['exponential_width'] = min(empty ** width, 1e15)  # Cap to avoid overflow

    return metrics

def collect_all_data(output_dir='output'):
    """Collect data from all output files."""
    all_data = []

    output_path = Path(output_dir)
    for algo_num in [1, 2, 3]:
        pattern = f'algo{algo_num}_*.txt'
        for filepath in output_path.glob(pattern):
            data = parse_output_file(filepath)
            if data['generated_nodes'] is not None:
                data['algorithm'] = f'algo{algo_num}'
                data['puzzle_name'] = filepath.stem.replace(f'algo{algo_num}_', '')
                data['theoretical'] = compute_theoretical_metrics(data)
                all_data.append(data)

    return all_data

def create_plots(all_data):
    """Create visualization plots."""

    # Separate data by algorithm
    algo1_data = [d for d in all_data if d['algorithm'] == 'algo1']
    algo2_data = [d for d in all_data if d['algorithm'] == 'algo2']
    algo3_data = [d for d in all_data if d['algorithm'] == 'algo3']

    # Define the 3 theoretical metrics to plot
    theoretical_metrics = [
        ('iw_complexity', 'Pieces × Empty Spaces × IW Width', 'metric1'),
        ('combined', 'Pieces × Steps × Empty Spaces × IW Width', 'metric2'),
        ('exponential_width', 'Empty Spaces ^ IW Width', 'metric3')
    ]

    # Create separate figure for each theoretical metric
    for metric_key, metric_label, metric_name in theoretical_metrics:
        fig, ax = plt.subplots(1, 1, figsize=(10, 8))

        # Plot all three algorithms on the same graph
        for algo_data, color, algo_name, marker in [
            (algo1_data, 'red', 'Algorithm 1 (No Dup Detection)', 'o'),
            (algo2_data, 'blue', 'Algorithm 2 (Radix Tree)', 's'),
            (algo3_data, 'green', 'Algorithm 3 (Iterative Width)', '^')
        ]:
            x_vals = []
            y_vals = []

            for d in algo_data:
                if metric_key in d['theoretical'] and d['theoretical'][metric_key]:
                    x_vals.append(d['theoretical'][metric_key])
                    y_vals.append(d['generated_nodes'])

            if x_vals:
                ax.scatter(x_vals, y_vals, alpha=0.7, s=150,
                          color=color, label=algo_name, marker=marker,
                          edgecolors='black', linewidth=1)

        ax.set_xlabel(f'Theoretical Complexity:\n{metric_label}', fontsize=13, fontweight='bold')
        ax.set_ylabel('Generated Nodes', fontsize=13, fontweight='bold')
        ax.set_title(f'Generated Nodes vs Theoretical Complexity', fontsize=14, fontweight='bold')
        ax.legend(fontsize=11, loc='upper left')
        ax.grid(True, alpha=0.3, linestyle='--')
        ax.set_xscale('log')
        ax.set_yscale('log')

        plt.tight_layout()

        # Save with metric-specific filename
        filename = f'performance_{metric_name}.png'
        plt.savefig(filename, dpi=300, bbox_inches='tight')
        print(f"Saved {filename}")
        plt.close()

def print_statistics(all_data):
    """Print summary statistics."""
    print("\n" + "="*80)
    print("PERFORMANCE ANALYSIS SUMMARY")
    print("="*80)

    for algo_name in ['algo1', 'algo2', 'algo3']:
        algo_data = [d for d in all_data if d['algorithm'] == algo_name]
        if not algo_data:
            continue

        print(f"\n{algo_name.upper()}:")
        print(f"  Total puzzles: {len(algo_data)}")

        generated = [d['generated_nodes'] for d in algo_data]
        expanded = [d['expanded_nodes'] for d in algo_data]
        duplicates = [d['duplicated_nodes'] for d in algo_data]
        exec_time = [d['execution_time'] for d in algo_data]

        print(f"  Generated nodes: min={min(generated):,}, max={max(generated):,}, avg={np.mean(generated):,.0f}")
        print(f"  Expanded nodes:  min={min(expanded):,}, max={max(expanded):,}, avg={np.mean(expanded):,.0f}")
        print(f"  Duplicated nodes: min={min(duplicates):,}, max={max(duplicates):,}, avg={np.mean(duplicates):,.0f}")
        print(f"  Execution time (s): min={min(exec_time):.6f}, max={max(exec_time):.2f}, avg={np.mean(exec_time):.3f}")

        if algo_name in ['algo2', 'algo3']:
            total_attempted = [g + d for g, d in zip(generated, duplicates)]
            efficiency = [g / t * 100 if t > 0 else 0 for g, t in zip(generated, total_attempted)]
            print(f"  Efficiency %: min={min(efficiency):.1f}%, max={max(efficiency):.1f}%, avg={np.mean(efficiency):.1f}%")

if __name__ == '__main__':
    print("Analyzing algorithm performance...")

    # Collect all data
    all_data = collect_all_data('output')

    if not all_data:
        print("Error: No output files found in 'output/' directory")
        exit(1)

    print(f"Loaded {len(all_data)} puzzle results")

    # Print statistics
    print_statistics(all_data)

    # Create plots
    print("\nGenerating plots...")
    create_plots(all_data)

    print("\nAnalysis complete!")
    print("Generated files:")
    print("  - performance_metric1.png (Pieces × Empty Spaces × IW Width)")
    print("  - performance_metric2.png (Pieces × Steps × Empty Spaces × IW Width)")
    print("  - performance_metric3.png (Empty Spaces ^ IW Width)")

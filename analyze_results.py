#!/usr/bin/env python3
"""
Statistical Analysis of Impassable Gate Puzzle Solver Results
Analyzes test results from output/ directory
"""

import os
import re
from collections import defaultdict
import statistics

def parse_result_file(filepath):
    """Parse a single result file and extract metrics"""
    data = {}
    try:
        with open(filepath, 'r') as f:
            content = f.read()

        # Extract metrics using regex
        data['solution_path'] = re.search(r'Solution path: (.+)', content)
        data['solution_path'] = data['solution_path'].group(1) if data['solution_path'] else None

        data['execution_time'] = float(re.search(r'Execution time: ([\d.]+)', content).group(1))
        data['expanded_nodes'] = int(re.search(r'Expanded nodes: (\d+)', content).group(1))
        data['generated_nodes'] = int(re.search(r'Generated nodes: (\d+)', content).group(1))
        data['duplicated_nodes'] = int(re.search(r'Duplicated nodes: (\d+)', content).group(1))
        data['memory_usage'] = int(re.search(r'Auxiliary memory usage \(bytes\): (\d+)', content).group(1))
        data['num_pieces'] = int(re.search(r'Number of pieces in the puzzle: (\d+)', content).group(1))
        data['solution_steps'] = int(re.search(r'Number of steps in solution: (\d+)', content).group(1))
        data['empty_spaces'] = int(re.search(r'Number of empty spaces: (\d+)', content).group(1))

        solved_by = re.search(r'Solved by IW\((\d+)\)', content)
        data['solved_by_iw'] = int(solved_by.group(1)) if solved_by else None

        nodes_per_sec = re.search(r'Number of nodes expanded per second: ([\d.]+)', content)
        data['nodes_per_second'] = float(nodes_per_sec.group(1)) if nodes_per_sec else None

        return data
    except Exception as e:
        print(f"Error parsing {filepath}: {e}")
        return None

def calculate_stats(values):
    """Calculate statistics for a list of values"""
    if not values:
        return {}
    return {
        'min': min(values),
        'max': max(values),
        'mean': statistics.mean(values),
        'median': statistics.median(values),
        'stdev': statistics.stdev(values) if len(values) > 1 else 0,
        'total': sum(values)
    }

def main():
    output_dir = 'output'

    # Group results by algorithm
    results = defaultdict(list)

    # Parse all result files
    for filename in os.listdir(output_dir):
        if filename.endswith('.txt'):
            filepath = os.path.join(output_dir, filename)

            # Extract algorithm name
            if filename.startswith('algo1_'):
                algo = 'Algorithm 1'
            elif filename.startswith('algo2_'):
                algo = 'Algorithm 2'
            elif filename.startswith('algo3_'):
                algo = 'Algorithm 3'
            else:
                continue

            data = parse_result_file(filepath)
            if data:
                results[algo].append({
                    'filename': filename,
                    'data': data
                })

    # Generate summary report
    print("=" * 80)
    print("STATISTICAL SUMMARY OF IMPASSABLE GATE PUZZLE SOLVER RESULTS")
    print("=" * 80)
    print()

    for algo in sorted(results.keys()):
        print(f"\n{'=' * 80}")
        print(f"{algo}")
        print(f"{'=' * 80}")
        print(f"Total test cases: {len(results[algo])}")
        print()

        # Collect metrics
        metrics = {
            'execution_time': [],
            'expanded_nodes': [],
            'generated_nodes': [],
            'duplicated_nodes': [],
            'memory_usage': [],
            'num_pieces': [],
            'solution_steps': [],
            'empty_spaces': [],
            'nodes_per_second': []
        }

        solved_by_iw = defaultdict(int)

        for result in results[algo]:
            data = result['data']
            for key in metrics:
                if data.get(key) is not None:
                    metrics[key].append(data[key])

            if data.get('solved_by_iw') is not None:
                solved_by_iw[data['solved_by_iw']] += 1

        # Print statistics for each metric
        print("EXECUTION TIME (seconds):")
        stats = calculate_stats(metrics['execution_time'])
        print(f"  Min:    {stats['min']:.6f}")
        print(f"  Max:    {stats['max']:.6f}")
        print(f"  Mean:   {stats['mean']:.6f}")
        print(f"  Median: {stats['median']:.6f}")
        print(f"  StdDev: {stats['stdev']:.6f}")
        print(f"  Total:  {stats['total']:.6f}")
        print()

        print("EXPANDED NODES:")
        stats = calculate_stats(metrics['expanded_nodes'])
        print(f"  Min:    {stats['min']:,}")
        print(f"  Max:    {stats['max']:,}")
        print(f"  Mean:   {stats['mean']:,.2f}")
        print(f"  Median: {stats['median']:,.0f}")
        print(f"  StdDev: {stats['stdev']:,.2f}")
        print(f"  Total:  {stats['total']:,}")
        print()

        print("GENERATED NODES:")
        stats = calculate_stats(metrics['generated_nodes'])
        print(f"  Min:    {stats['min']:,}")
        print(f"  Max:    {stats['max']:,}")
        print(f"  Mean:   {stats['mean']:,.2f}")
        print(f"  Median: {stats['median']:,.0f}")
        print(f"  StdDev: {stats['stdev']:,.2f}")
        print(f"  Total:  {stats['total']:,}")
        print()

        print("DUPLICATED NODES:")
        stats = calculate_stats(metrics['duplicated_nodes'])
        print(f"  Min:    {stats['min']:,}")
        print(f"  Max:    {stats['max']:,}")
        print(f"  Mean:   {stats['mean']:,.2f}")
        print(f"  Median: {stats['median']:,.0f}")
        print(f"  StdDev: {stats['stdev']:,.2f}")
        print(f"  Total:  {stats['total']:,}")
        print()

        print("AUXILIARY MEMORY USAGE (bytes):")
        stats = calculate_stats(metrics['memory_usage'])
        print(f"  Min:    {stats['min']:,}")
        print(f"  Max:    {stats['max']:,}")
        print(f"  Mean:   {stats['mean']:,.2f}")
        print(f"  Median: {stats['median']:,.0f}")
        print(f"  StdDev: {stats['stdev']:,.2f}")
        print(f"  Total:  {stats['total']:,} ({stats['total']/1024:.2f} KB)")
        print()

        print("SOLUTION STEPS:")
        stats = calculate_stats(metrics['solution_steps'])
        print(f"  Min:    {stats['min']}")
        print(f"  Max:    {stats['max']}")
        print(f"  Mean:   {stats['mean']:.2f}")
        print(f"  Median: {stats['median']:.0f}")
        print(f"  StdDev: {stats['stdev']:.2f}")
        print()

        print("NODES EXPANDED PER SECOND:")
        if metrics['nodes_per_second']:
            stats = calculate_stats(metrics['nodes_per_second'])
            print(f"  Min:    {stats['min']:,.2f}")
            print(f"  Max:    {stats['max']:,.2f}")
            print(f"  Mean:   {stats['mean']:,.2f}")
            print(f"  Median: {stats['median']:,.2f}")
            print(f"  StdDev: {stats['stdev']:,.2f}")
        else:
            print("  No data available")
        print()

        print("SOLVED BY IW WIDTH:")
        for width in sorted(solved_by_iw.keys()):
            count = solved_by_iw[width]
            percentage = (count / len(results[algo])) * 100
            print(f"  IW({width}): {count} puzzles ({percentage:.1f}%)")
        print()

    # Cross-algorithm comparison
    print(f"\n{'=' * 80}")
    print("CROSS-ALGORITHM COMPARISON")
    print(f"{'=' * 80}")
    print()

    comparison_metrics = ['execution_time', 'expanded_nodes', 'memory_usage']

    for metric in comparison_metrics:
        print(f"{metric.upper().replace('_', ' ')}:")
        for algo in sorted(results.keys()):
            values = []
            for result in results[algo]:
                if result['data'].get(metric) is not None:
                    values.append(result['data'][metric])

            if values:
                mean_val = statistics.mean(values)
                if metric == 'execution_time':
                    print(f"  {algo}: {mean_val:.6f} seconds (mean)")
                elif metric == 'memory_usage':
                    print(f"  {algo}: {mean_val:,.2f} bytes (mean)")
                else:
                    print(f"  {algo}: {mean_val:,.2f} (mean)")
        print()

    print("=" * 80)

if __name__ == "__main__":
    main()

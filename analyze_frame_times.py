#!/usr/bin/env python3
"""
Frame time analysis script for Aurora engine performance data
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import argparse
from pathlib import Path

def print_stats(label, series, unit='ms', indent=''):
    """Helper to print Avg, Min, Max, Std Dev for a data series"""
    if len(series) == 0:
        print(f"{indent}{label}: No data")
        return
        
    avg = series.mean()
    minimum = series.min()
    maximum = series.max()
    std = series.std()
    
    # Format based on unit
    if unit == 'ms':
        print(f"{indent}{label:<20} Avg: {avg:6.3f} ms | Min: {minimum:6.3f} ms | Max: {maximum:6.3f} ms | Std: {std:5.3f} ms")
    elif unit == '':
        print(f"{indent}{label:<20} Avg: {avg:6.1f}    | Min: {minimum:6.0f}    | Max: {maximum:6.0f}    | Std: {std:5.1f}")
    else:
        print(f"{indent}{label:<20} Avg: {avg:.3f} {unit} | Min: {minimum:.3f} {unit} | Max: {maximum:.3f} {unit} | Std: {std:.3f} {unit}")

def analyze_frame_times(csv_file, skip_count=60):
    """Analyze frame time data and generate performance insights"""
    
    # Read the CSV data
    df = pd.read_csv(csv_file)
    
    # Skip the first N points (initialization noise)
    if len(df) > skip_count:
        df = df.iloc[skip_count:].reset_index(drop=True)
        print(f"Note: Skipped first {skip_count} data points (initialization)")
    else:
        print(f"Warning: Not enough data points to skip {skip_count} frames. Using all available data.")
    
    print("=== Aurora Engine Frame Time Analysis ===\n")
    
    # Basic statistics
    print("Basic Statistics:")
    print(f"Total frames: {len(df)}")
    print(f"Runtime: {df['timestamp_ms'].iloc[-1] / 1000:.2f} seconds")
    print(f"Average FPS: {df['fps'].mean():.2f}")
    
    print_stats("Frame Time", df['frame_time_ms'])

    if 'render_components_ms' in df.columns:
         print("\nRender Stats:")
         print_stats("Render Time", df['render_components_ms'])
    
    if 'draw_calls' in df.columns:
        print("\nDraw Calls Stats:")
        print_stats("Draw Calls", df['draw_calls'], unit='')

    if 'begin_frame_ms' in df.columns:
        print("\nPipeline Stats:")
        print_stats("Poll Events", df['poll_events_ms'])
        print_stats("Begin Frame", df['begin_frame_ms'])
        print_stats("End Frame", df['end_frame_ms'])

    if 'profiler_ui_ms' in df.columns:
        print("\nProfiler Internal Stats:")
        print_stats("Profiler UI Update", df['profiler_ui_ms'])

    if 'text_update_ms' in df.columns:
         print("\nComponent Stats:")
         print_stats("AuroraText::setText", df['text_update_ms'])
    
    # Calculate Unaccounted time
    # Frame Time - (Render + Begin + End + Poll + UI)
    known_time = pd.Series(0.0, index=df.index)
    if 'render_components_ms' in df.columns: known_time += df['render_components_ms']
    if 'begin_frame_ms' in df.columns: known_time += df['begin_frame_ms']
    if 'end_frame_ms' in df.columns: known_time += df['end_frame_ms']
    if 'poll_events_ms' in df.columns: known_time += df['poll_events_ms']
    if 'profiler_ui_ms' in df.columns: known_time += df['profiler_ui_ms']
    
    df['unaccounted_ms'] = df['frame_time_ms'] - known_time
    # Clamp to 0 (sometimes timers might slightly mismatch)
    df['unaccounted_ms'] = df['unaccounted_ms'].clip(lower=0) 
    
    print("\nUnaccounted Time (Gap):")
    print_stats("Unaccounted", df['unaccounted_ms'])

    print()
    
    # Performance percentiles
    print("Performance Percentiles:")
    percentiles = [50, 90, 95, 99]
    for p in percentiles:
        frame_time_p = np.percentile(df['frame_time_ms'], p)
        fps_p = 1000.0 / frame_time_p
        print(f"{p}th percentile: {frame_time_p:.3f} ms ({fps_p:.1f} FPS)")
    print()
    
    # Frame drops (assuming 60 FPS target)
    target_frame_time = 1000.0 / 60.0  # 16.67ms for 60 FPS
    frame_drops = df[df['frame_time_ms'] > target_frame_time]
    print(f"Frame drops (>{target_frame_time:.2f}ms): {len(frame_drops)} ({len(frame_drops)/len(df)*100:.2f}%)")
    
    # Detect stutters (frame times > 2x average)
    avg_frame_time = df['frame_time_ms'].mean()
    stutters = df[df['frame_time_ms'] > 2 * avg_frame_time]
    print(f"Stutters (>2x avg): {len(stutters)} ({len(stutters)/len(df)*100:.2f}%)")
    print()
    
    # Determine plot layout based on available data
    has_render_stats = 'render_components_ms' in df.columns and 'draw_calls' in df.columns
    has_detailed_stats = 'begin_frame_ms' in df.columns
    
    rows = 3 if has_render_stats else 2
    cols = 2
    
    fig, axes = plt.subplots(rows, cols, figsize=(15, 5 * rows))
    fig.suptitle('Aurora Engine Performance Analysis', fontsize=16)
    
    # Frame time over time
    axes[0, 0].plot(df['timestamp_ms'] / 1000, df['frame_time_ms'], alpha=0.7)
    axes[0, 0].set_xlabel('Time (seconds)')
    axes[0, 0].set_ylabel('Frame Time (ms)')
    axes[0, 0].set_title('Frame Time Over Time')
    axes[0, 0].grid(True, alpha=0.3)
    
    # FPS over time
    axes[0, 1].plot(df['timestamp_ms'] / 1000, df['fps'], alpha=0.7, color='green')
    axes[0, 1].set_xlabel('Time (seconds)')
    axes[0, 1].set_ylabel('FPS')
    axes[0, 1].set_title('FPS Over Time')
    axes[0, 1].grid(True, alpha=0.3)
    
    # Frame time histogram
    axes[1, 0].hist(df['frame_time_ms'], bins=50, alpha=0.7, color='orange')
    axes[1, 0].axvline(x=df['frame_time_ms'].mean(), color='g', linestyle='--', label='Average')
    axes[1, 0].set_xlabel('Frame Time (ms)')
    axes[1, 0].set_ylabel('Frequency')
    axes[1, 0].set_title('Frame Time Distribution')
    axes[1, 0].grid(True, alpha=0.3)
    
    # Rolling average
    window_size = 60  # 1 second at 60 FPS
    if len(df) > window_size:
        rolling_avg = df['frame_time_ms'].rolling(window=window_size).mean()
        axes[1, 1].plot(df['timestamp_ms'] / 1000, rolling_avg, color='purple')
        axes[1, 1].set_xlabel('Time (seconds)')
        axes[1, 1].set_ylabel('Frame Time (ms)')
        axes[1, 1].set_title(f'Rolling Average ({window_size} frames)')
        axes[1, 1].grid(True, alpha=0.3)
    
    # New plots for Render Stats
    if has_render_stats:
        # Draw Calls
        axes[2, 0].plot(df['timestamp_ms'] / 1000, df['draw_calls'], alpha=0.7, color='brown')
        axes[2, 0].set_xlabel('Time (seconds)')
        axes[2, 0].set_ylabel('Draw Calls')
        axes[2, 0].set_title('Draw Calls Over Time')
        axes[2, 0].grid(True, alpha=0.3)

        if has_detailed_stats:
            # Stacked Plot for Time Breakdown
            labels = ['Poll Events', 'Begin Frame', 'Render Components', 'End Frame']
            data = [
                df['poll_events_ms'],
                df['begin_frame_ms'],
                df['render_components_ms'],
                df['end_frame_ms']
            ]
            
            if 'profiler_ui_ms' in df.columns:
                labels.append('Profiler UI')
                data.append(df['profiler_ui_ms'])
                
            labels.append('Unaccounted')
            data.append(df['unaccounted_ms'])
            
            axes[2, 1].stackplot(df['timestamp_ms'] / 1000, data, labels=labels, alpha=0.7)
            axes[2, 1].plot(df['timestamp_ms'] / 1000, df['frame_time_ms'], color='black', linestyle='--', label='Total Frame Time', alpha=0.5)
            
            axes[2, 1].set_xlabel('Time (seconds)')
            axes[2, 1].set_ylabel('Time (ms)')
            axes[2, 1].set_title('Frame Time Breakdown (Stacked)')
            axes[2, 1].legend(loc='upper left')
            axes[2, 1].grid(True, alpha=0.3)
        else:
            # Render Time vs Frame Time (Legacy)
            axes[2, 1].plot(df['timestamp_ms'] / 1000, df['frame_time_ms'], alpha=0.5, label='Total Frame Time')
            axes[2, 1].plot(df['timestamp_ms'] / 1000, df['render_components_ms'], alpha=0.7, color='red', label='Render Components')
            axes[2, 1].set_xlabel('Time (seconds)')
            axes[2, 1].set_ylabel('Time (ms)')
            axes[2, 1].set_title('Render Time vs Total Frame Time')
            axes[2, 1].legend()
            axes[2, 1].grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()

    # Create a second figure for focused analysis (Zoomed View)
    fig2, axes2 = plt.subplots(2, 1, figsize=(15, 10))
    fig2.suptitle('Focused Analysis (Last 200 Frames & Scatter)', fontsize=16)

    # 1. Scatter Plot (good for seeing density and outliers without lines connecting them)
    axes2[0].scatter(df['timestamp_ms'] / 1000, df['frame_time_ms'], alpha=0.5, s=2)
    axes2[0].set_xlabel('Time (seconds)')
    axes2[0].set_ylabel('Frame Time (ms)')
    axes2[0].set_title('Frame Time Scatter Plot')
    axes2[0].grid(True, alpha=0.3)

    # 2. Zoomed View (Last 200 frames)
    if len(df) > 200:
        last_200 = df.iloc[-200:]
        axes2[1].plot(last_200['timestamp_ms'] / 1000, last_200['frame_time_ms'], marker='o', markersize=3)
        axes2[1].set_xlabel('Time (seconds)')
        axes2[1].set_ylabel('Frame Time (ms)')
        axes2[1].set_title('Last 200 Frames (Zoomed Pattern)')
        axes2[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.show()

    # Create a third figure for Stacked Analysis
    fig3, axes3 = plt.subplots(2, 1, figsize=(15, 10))
    fig3.suptitle('Stacked Analysis (Full & Zoomed)', fontsize=16)

    # Prepare data for stacking
    labels = ['Poll Events', 'Begin Frame', 'Render Components', 'End Frame']
    data = [
        df['poll_events_ms'],
        df['begin_frame_ms'],
        df['render_components_ms'],
        df['end_frame_ms']
    ]
    
    if 'profiler_ui_ms' in df.columns:
        labels.append('Profiler UI')
        data.append(df['profiler_ui_ms'])
        
    labels.append('Unaccounted')
    data.append(df['unaccounted_ms'])

    # 1. Full Stacked Chart
    axes3[0].stackplot(df['timestamp_ms'] / 1000, data, labels=labels, alpha=0.7)
    axes3[0].plot(df['timestamp_ms'] / 1000, df['frame_time_ms'], color='black', linestyle='--', label='Total Frame Time', alpha=0.5)
    axes3[0].set_xlabel('Time (seconds)')
    axes3[0].set_ylabel('Time (ms)')
    axes3[0].set_title('Full Frame Time Breakdown (Stacked)')
    axes3[0].legend(loc='upper left')
    axes3[0].grid(True, alpha=0.3)

    # 2. Zoomed Stacked Chart (Last 200 frames)
    if len(df) > 200:
        last_200 = df.iloc[-200:]
        
        data_200 = [
            last_200['poll_events_ms'],
            last_200['begin_frame_ms'],
            last_200['render_components_ms'],
            last_200['end_frame_ms']
        ]
        
        if 'profiler_ui_ms' in df.columns:
            data_200.append(last_200['profiler_ui_ms'])
            
        data_200.append(last_200['unaccounted_ms'])
        
        axes3[1].stackplot(last_200['timestamp_ms'] / 1000, data_200, labels=labels, alpha=0.7)
        axes3[1].plot(last_200['timestamp_ms'] / 1000, last_200['frame_time_ms'], color='black', linestyle='--', label='Total Frame Time', alpha=0.5)
        axes3[1].set_xlabel('Time (seconds)')
        axes3[1].set_ylabel('Time (ms)')
        axes3[1].set_title('Last 200 Frames Breakdown (Zoomed Stacked)')
        # axes3[1].legend(loc='upper left') # Legend already on top plot
        axes3[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.show()

    # --- New Analysis: Grouping and Periodicity ---
    print("\n=== Frame Group Analysis ===")
    
    # Define groups based on user observations
    # Group 1: ~0.2ms (Ultra Fast)
    # Group 2: ~0.8ms (Fast)
    # Group 3: ~2.1ms (Medium)
    # Group 4: > 4ms (Spikes)
    
    bins = [0, 0.5, 1.5, 3.0, float('inf')]
    labels = ['Ultra Fast (~0.2ms)', 'Fast (~0.8ms)', 'Medium (~2.1ms)', 'Spikes (>3.0ms)']
    
    df['group'] = pd.cut(df['frame_time_ms'], bins=bins, labels=labels)
    
    # Calculate stats per group
    grouped = df.groupby('group', observed=True)
    
    stats_cols = ['frame_time_ms']
    if 'render_components_ms' in df.columns:
        stats_cols.append('render_components_ms')
    if 'begin_frame_ms' in df.columns:
        stats_cols.append('begin_frame_ms')
    if 'end_frame_ms' in df.columns:
        stats_cols.append('end_frame_ms')
    if 'profiler_ui_ms' in df.columns:
        stats_cols.append('profiler_ui_ms')
            
    if 'text_update_ms' in df.columns:
        stats_cols.append('text_update_ms')

    if 'unaccounted_ms' in df.columns:
        stats_cols.append('unaccounted_ms')

    for name, group in grouped:
        if len(group) == 0:
            continue
            
        print(f"\nGroup: {name}")
        print(f"  Count: {len(group)} ({len(group)/len(df)*100:.1f}%)")
        
        for col in stats_cols:
            unit = '' if col == 'draw_calls' else 'ms'
            print_stats(col, group[col], unit=unit, indent='  ')

    # --- Periodicity Analysis for Spikes ---
    print("\n=== Periodicity Analysis (Spikes) ===")
    spike_df = df[df['group'] == 'Spikes (>3.0ms)']
    
    if len(spike_df) > 2:
        # Calculate frame intervals (difference in index)
        indices = spike_df.index.to_series()
        intervals = indices.diff().dropna()
        
        avg_interval = intervals.mean()
        std_interval = intervals.std()
        mode_interval = intervals.mode()[0] if not intervals.mode().empty else 0
        
        print(f"Spike Intervals (Frames):")
        print(f"  Average Distance: {avg_interval:.2f} frames")
        print(f"  Most Common Distance (Mode): {mode_interval} frames")
        print(f"  Std Dev: {std_interval:.2f}")
        
        if std_interval < 1.0:
            print("  -> DETERMINISTIC PERIODICITY DETECTED!")
        elif std_interval < avg_interval * 0.2:
             print("  -> STRONG PERIODICITY DETECTED")
        else:
             print("  -> Irregular/Noisy distribution")

        # Time intervals
        timestamps = spike_df['timestamp_ms']
        time_diffs = timestamps.diff().dropna()
        print(f"Spike Intervals (Time):")
        print(f"  Average Time Diff: {time_diffs.mean():.2f} ms")
        
    else:
        print("Not enough spikes to analyze periodicity.")

def main():
    parser = argparse.ArgumentParser(description='Analyze Aurora engine frame time data')
    parser.add_argument('csv_file', type=str, help='Path to the frame_times.csv file')
    parser.add_argument('--skip', type=int, default=30, help='Number of initial frames to skip (default: 30)')
    
    args = parser.parse_args()
    
    csv_path = Path(args.csv_file)
    if not csv_path.exists():
        print(f"Error: File {csv_path} not found")
        return
    
    analyze_frame_times(csv_path, args.skip)

if __name__ == "__main__":
    main()

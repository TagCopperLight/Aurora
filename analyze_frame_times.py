#!/usr/bin/env python3
"""
Frame time analysis script for Aurora engine performance data
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import argparse
from pathlib import Path

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
    print(f"Average frame time: {df['frame_time_ms'].mean():.3f} ms")
    print(f"Min frame time: {df['frame_time_ms'].min():.3f} ms")
    print(f"Max frame time: {df['frame_time_ms'].max():.3f} ms")
    print(f"Frame time std dev: {df['frame_time_ms'].std():.3f} ms")

    if 'render_components_ms' in df.columns:
         print("\nRender Stats:")
         print(f"Average Render Time: {df['render_components_ms'].mean():.3f} ms")
         print(f"Max Render Time: {df['render_components_ms'].max():.3f} ms")
         print(f"Min Render Time: {df['render_components_ms'].min():.3f} ms")
         print(f"Render Time std dev: {df['render_components_ms'].std():.3f} ms")
    
    if 'draw_calls' in df.columns:
        print("\nDraw Calls Stats:")
        print(f"Average Draw Calls: {df['draw_calls'].mean():.1f}")
        print(f"Max Draw Calls: {df['draw_calls'].max()}")
        print(f"Min Draw Calls: {df['draw_calls'].min()}")
        print(f"Draw Calls std dev: {df['draw_calls'].std():.1f}")

    if 'begin_frame_ms' in df.columns:
        print("\nPipeline Stats (Avg):")
        print(f"Poll Events: {df['poll_events_ms'].mean():.3f} ms")
        print(f"Begin Frame: {df['begin_frame_ms'].mean():.3f} ms")
        print(f"End Frame: {df['end_frame_ms'].mean():.3f} ms")

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
            
            axes[2, 1].stackplot(df['timestamp_ms'] / 1000, data, labels=labels, alpha=0.7)
            # axes[2, 1].plot(df['timestamp_ms'] / 1000, df['frame_time_ms'], color='black', linestyle='--', label='Total Frame Time', alpha=0.5)
            
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

#!/usr/bin/env python3
"""
Frame time analysis script for Aurora engine performance data
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import argparse
from pathlib import Path

def analyze_frame_times(csv_file):
    """Analyze frame time data and generate performance insights"""
    
    # Read the CSV data
    df = pd.read_csv(csv_file)
    
    # Skip the first 10 points (initialization noise)
    if len(df) > 10:
        df = df.iloc[10:].reset_index(drop=True)
        print("Note: Skipped first 10 data points (initialization)")
    
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
    
    # Generate plots
    fig, axes = plt.subplots(2, 2, figsize=(15, 10))
    fig.suptitle('Aurora Engine Performance Analysis', fontsize=16)
    
    # Frame time over time
    axes[0, 0].plot(df['timestamp_ms'] / 1000, df['frame_time_ms'], alpha=0.7)
    # axes[0, 0].axhline(y=target_frame_time, color='r', linestyle='--', label='60 FPS target')
    axes[0, 0].set_xlabel('Time (seconds)')
    axes[0, 0].set_ylabel('Frame Time (ms)')
    axes[0, 0].set_title('Frame Time Over Time')
    # axes[0, 0].legend()
    axes[0, 0].grid(True, alpha=0.3)
    
    # FPS over time
    axes[0, 1].plot(df['timestamp_ms'] / 1000, df['fps'], alpha=0.7, color='green')
    # axes[0, 1].axhline(y=60, color='r', linestyle='--', label='60 FPS target')
    axes[0, 1].set_xlabel('Time (seconds)')
    axes[0, 1].set_ylabel('FPS')
    axes[0, 1].set_title('FPS Over Time')
    # axes[0, 1].legend()
    axes[0, 1].grid(True, alpha=0.3)
    
    # Frame time histogram
    axes[1, 0].hist(df['frame_time_ms'], bins=50, alpha=0.7, color='orange')
    # axes[1, 0].axvline(x=target_frame_time, color='r', linestyle='--', label='60 FPS target')
    axes[1, 0].axvline(x=df['frame_time_ms'].mean(), color='g', linestyle='--', label='Average')
    axes[1, 0].set_xlabel('Frame Time (ms)')
    axes[1, 0].set_ylabel('Frequency')
    axes[1, 0].set_title('Frame Time Distribution')
    # axes[1, 0].legend()
    axes[1, 0].grid(True, alpha=0.3)
    
    # Rolling average (smoothed performance)
    window_size = 60  # 1 second at 60 FPS
    if len(df) > window_size:
        rolling_avg = df['frame_time_ms'].rolling(window=window_size).mean()
        axes[1, 1].plot(df['timestamp_ms'] / 1000, rolling_avg, color='purple')
        # axes[1, 1].axhline(y=target_frame_time, color='r', linestyle='--', label='60 FPS target')
        axes[1, 1].set_xlabel('Time (seconds)')
        axes[1, 1].set_ylabel('Frame Time (ms)')
        axes[1, 1].set_title(f'Rolling Average ({window_size} frames)')
        # axes[1, 1].legend()
        axes[1, 1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    # Save the plot
    # output_file = csv_file.stem + '_analysis.png'
    # plt.savefig(output_file, dpi=300, bbox_inches='tight')
    # print(f"Performance analysis plot saved to: {output_file}")
    
    # Show the plot
    plt.show()

def main():
    parser = argparse.ArgumentParser(description='Analyze Aurora engine frame time data')
    parser.add_argument('csv_file', type=str, help='Path to the frame_times.csv file')
    parser.add_argument('--no-plot', action='store_true', help='Skip generating plots')
    
    args = parser.parse_args()
    
    csv_path = Path(args.csv_file)
    if not csv_path.exists():
        print(f"Error: File {csv_path} not found")
        return
    
    analyze_frame_times(csv_path)

if __name__ == "__main__":
    main()

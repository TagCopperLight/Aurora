#!/usr/bin/env python3
"""
A/B benchmark runner for Aurora.

Runs several aurora_bench builds interleaved (A, B, A, B, ...) so that thermal
and compositor noise affects every variant equally, then reports the median of
each metric per variant and the relative change versus the first variant.

Usage:
    bench/compare.py name=path/to/aurora_bench[@extra args] [name=path ...]

    e.g. bench/compare.py msaa8=build/aurora_bench/aurora_bench "msaa4=build/aurora_bench/aurora_bench@--msaa 4"
                     [--scenes static,dynamic,panel] [--rounds 5]
                     [--frames 1500] [--warmup 200] [--json out.json]
"""

import argparse
import json
import os
import statistics
import subprocess
import sys

METRICS = [
    ("frame_ms_mean", "frame ms (mean)"),
    ("frame_ms_p99", "frame ms (p99)"),
    ("cpu_work_ms_mean", "CPU work ms"),
    ("process_cpu_ms_per_frame", "process CPU ms/frame"),
    ("render_components_ms", "record ms"),
    ("update_ms", "update ms"),
    ("end_frame_ms", "submit+present ms"),
    ("gpu_frame_ms", "GPU ms"),
    ("fps", "FPS"),
    ("setup_ms", "setup ms"),
    ("gpu_mem_mib", "GPU memory MiB"),
    ("first_frame_ms", "time to 1st frame ms"),
]

HIGHER_IS_BETTER = {"fps"}


def run_once(binary, scene, frames, warmup, max_seconds, extra_args=()):
    # The binary loads shaders/ and assets/ relative to its build directory
    cwd = os.path.dirname(os.path.dirname(os.path.abspath(binary)))
    cmd = [binary, "--scene", scene, "--frames", str(frames), "--warmup", str(warmup),
           "--max-seconds", str(max_seconds), *extra_args]
    try:
        out = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=max_seconds + 30)
    except subprocess.TimeoutExpired:
        print(f"  !! {binary} {scene}: timed out", file=sys.stderr)
        return None
    for line in out.stdout.splitlines():
        if line.startswith("{"):
            data = json.loads(line)
            if data.get("frames", 0) == 0:
                return None
            return data
    print(f"  !! {binary} {scene}: no result (rc={out.returncode})\n{out.stderr[-2000:]}", file=sys.stderr)
    return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("variants", nargs="+", help="name=path/to/aurora_bench")
    parser.add_argument("--scenes", default="static,dynamic,panel")
    parser.add_argument("--rounds", type=int, default=5)
    parser.add_argument("--frames", type=int, default=1500)
    parser.add_argument("--warmup", type=int, default=200)
    parser.add_argument("--max-seconds", type=float, default=60.0)
    parser.add_argument("--json", help="write raw results to this file")
    args = parser.parse_args()

    variants = []
    for spec in args.variants:
        name, _, rest = spec.partition("=")
        path, _, extra = rest.partition("@")
        variants.append((name, path, tuple(extra.split())))

    scenes = args.scenes.split(",")
    results = {scene: {name: [] for name, _, _ in variants} for scene in scenes}

    for r in range(args.rounds):
        for scene in scenes:
            # Alternate order each round to cancel out ordering effects
            order = variants if r % 2 == 0 else list(reversed(variants))
            for name, path, extra in order:
                data = run_once(path, scene, args.frames, args.warmup, args.max_seconds, extra)
                if data:
                    results[scene][name].append(data)
                    print(f"round {r + 1}/{args.rounds} {scene:8s} {name:12s} "
                          f"frame={data['frame_ms_mean']:.3f}ms cpu={data['cpu_work_ms_mean']:.3f}ms "
                          f"gpu={data['gpu_frame_ms']:.3f}ms", flush=True)

    base = variants[0][0]
    print()
    for scene in scenes:
        print(f"=== scene: {scene} (median of {args.rounds} runs) ===")
        header = f"{'metric':24s}" + "".join(f"{name:>22s}" for name, _, _ in variants)
        print(header)
        for key, label in METRICS:
            row = f"{label:24s}"
            # Older builds may not report every metric
            base_runs = [d for d in results[scene][base] if key in d]
            base_val = statistics.median(d[key] for d in base_runs) if base_runs else None
            for name, _, _ in variants:
                runs = [d for d in results[scene][name] if key in d]
                if not runs:
                    row += f"{'n/a':>22s}"
                    continue
                val = statistics.median(d[key] for d in runs)
                if name == base or not base_val:
                    row += f"{val:>22.3f}"
                else:
                    change = (val - base_val) / base_val * 100.0
                    row += f"{val:>12.3f} ({change:+6.1f}%)"
            print(row)
        print()

    if args.json:
        with open(args.json, "w") as f:
            json.dump(results, f, indent=1)


if __name__ == "__main__":
    main()

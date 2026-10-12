# Aurora benchmarks

`aurora_bench` (in `aurora_bench/`) renders a stress scene with the frame limiter off and prints one
JSON line of timings: frame time, per-phase CPU time (from `AURORA_PROFILE` scopes), GPU time
(timestamp queries), process CPU per frame, GPU memory (DRM fdinfo) and time to first frame.

```
aurora_bench --scene static|dynamic|panel [--frames N] [--warmup N] [--max-seconds S]
             [--msaa 2|4|8] [--capture out.ppm] [--verbose]
```

- `static`: 60 cards, 400 rounded rects, 300 circles, 1200 text labels
- `dynamic`: same scene, 600 labels re-laid out and 400 rects moved every frame
- `panel`: a small `AuroraPanel` with every value updated each frame (like `debug_example`)

It opens a fullscreen window and always closes itself after `--max-seconds` (default 60).
Run it from the build directory (it loads `shaders/` and `assets/` relatively).

## Comparing variants

```
bench/snapshot_build.sh <name> [build type]   # snapshot the current tree into /tmp/aurora-variants/<name> and build it (Release by default, "" for the default dev config)
bench/compare.py a=/tmp/aurora-variants/a/build/aurora_bench/aurora_bench \
                 "b=/tmp/aurora-variants/b/build/aurora_bench/aurora_bench@--msaa 4" --rounds 5
bench/check_render.sh <reference aurora_bench> <candidate aurora_bench>   # capture + pixel diff every scene
bench/sync_infra.sh <name>...                 # copy benchmark/profiling infrastructure into older snapshots
```

`compare.py` runs the variants interleaved (alternating order every round) and reports medians and the
change relative to the first variant. Frames are usually GPU-bound, so look at process CPU and GPU time
as well as frame time.

#!/usr/bin/env bash
# Capture the final frame of each scene for a reference and a candidate build, then diff them.
# Usage: bench/check_render.sh <reference aurora_bench> <candidate aurora_bench> [scenes...]
set -uo pipefail
ref="$1"; cand="$2"; shift 2
scenes=("${@:-static dynamic panel}")
[ $# -eq 0 ] && scenes=(static dynamic panel)
out="${AURORA_CAPTURES:-/tmp/aurora-captures}"
mkdir -p "$out"
status=0
for scene in "${scenes[@]}"; do
    for side in ref cand; do
        bin="$ref"; [ "$side" = cand ] && bin="$cand"
        dir="$(dirname "$(dirname "$(realpath "$bin")")")"
        (cd "$dir" && timeout 60 "$bin" --scene "$scene" --frames 30 --warmup 10 --max-seconds 30 --capture "$out/$scene-$side.ppm" > /dev/null 2>&1)
    done
    python3 "$(dirname "$0")/image_diff.py" "$out/$scene-ref.ppm" "$out/$scene-cand.ppm" "$out/$scene-diff.png" || status=1
done
exit $status

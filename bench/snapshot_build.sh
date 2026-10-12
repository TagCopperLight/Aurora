#!/usr/bin/env bash
# Snapshot the current source tree into $AURORA_VARIANTS/<name> and build it.
# Usage: bench/snapshot_build.sh <name> [cmake build type] [extra cmake args...]
# The resulting benchmark binary is $AURORA_VARIANTS/<name>/build/aurora_bench/aurora_bench
set -euo pipefail

name="$1"; shift
build_type="${1-Release}"; shift || true
root="$(cd "$(dirname "$0")/.." && pwd)"
variants="${AURORA_VARIANTS:-/tmp/aurora-variants}"
dest="$variants/$name"

mkdir -p "$dest"
rsync -a --delete \
    --exclude '/build*' --exclude '/.git' --exclude '/.cache' --exclude '/compile_commands.json' \
    "$root/" "$dest/src/"

cmake -S "$dest/src" -B "$dest/build" -DCMAKE_BUILD_TYPE="$build_type" -DAURORA_ENABLE_GPROF=OFF "$@" > "$dest/cmake.log"
make -C "$dest/build" -j"$(nproc)" aurora_bench > "$dest/make.log" 2>&1 || { tail -30 "$dest/make.log"; exit 1; }
echo "$dest/build/aurora_bench/aurora_bench"

#!/usr/bin/env bash
# Copy benchmark/profiling infrastructure files from the working tree into an existing
# snapshot (so older variants can be measured with the same harness) and rebuild it.
# Usage: bench/sync_infra.sh <variant-name>...
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
variants="${AURORA_VARIANTS:-/tmp/aurora-variants}"
files=(
    aurora_bench/src/main.cpp
    aurora_engine/include/aurora_engine/core/aurora_renderer.hpp
    aurora_engine/src/core/aurora_renderer.cpp
    aurora_engine/include/aurora_engine/core/aurora_swap_chain.hpp
    aurora_engine/include/aurora_engine/core/aurora_window.hpp
    aurora_ui/include/aurora_ui/aurora_ui.hpp
    aurora_ui/src/aurora_ui.cpp
    aurora_ui/include/aurora_ui/components/aurora_component_interface.hpp
)
for name in "$@"; do
    for f in "${files[@]}"; do cp "$root/$f" "$variants/$name/src/$f"; done
    make -C "$variants/$name/build" -j"$(nproc)" aurora_bench > "$variants/$name/make.log" 2>&1 || { tail -30 "$variants/$name/make.log"; exit 1; }
    echo "synced $name"
done

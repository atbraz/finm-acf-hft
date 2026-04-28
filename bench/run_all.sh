#!/usr/bin/env bash
# Build each benchmark variant in its own build dir and run it.
set -euo pipefail

CXX="${CXX:-/opt/homebrew/opt/llvm/bin/clang++}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
RESULTS="$ROOT/results/all_runs.csv"

mkdir -p "$ROOT/results"
rm -f "$RESULTS"

run_variant() {
    local label="$1"; shift
    local build_dir="$ROOT/build-$label"
    cmake -B "$build_dir" -G Ninja \
        -DCMAKE_CXX_COMPILER="$CXX" \
        -DCMAKE_BUILD_TYPE=Release \
        "$@" >/dev/null
    cmake --build "$build_dir" >/dev/null
    "$build_dir/hft_bench" --ticks 10000 --seed 42 --label "$label" --out "$RESULTS"
}

# Reference build (all toggles at default: pool on, alignas on, flat book, smart ptr)
run_variant reference

# Pointer ownership
run_variant raw-ptr -DHFT_USE_RAW_PTR=ON

# Cache alignment
run_variant no-align -DHFT_ALIGN_CACHE=OFF

# Allocator
run_variant no-pool -DHFT_USE_POOL=OFF

# Book container
run_variant book-mmap -DHFT_BOOK_IMPL_FLAT=OFF

# Load scaling (uses reference build, varies tick count)
"$ROOT/build-reference/hft_bench" --ticks   1000 --seed 42 --label load-1k   --out "$RESULTS"
"$ROOT/build-reference/hft_bench" --ticks  10000 --seed 42 --label load-10k  --out "$RESULTS"
"$ROOT/build-reference/hft_bench" --ticks 100000 --seed 42 --label load-100k --out "$RESULTS"

echo "Results in $RESULTS"

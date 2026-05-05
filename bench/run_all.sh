#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

out=results/orderbook_add.csv
mkdir -p results
echo "label,orders,seed,total_ns,ns_per_op" > "$out"
for variant in "" "reserve"; do
    for n in 1000 5000 10000 50000 100000; do
        for seed in 1 2 3; do
            ./build/hft_bench "$n" "$seed" $variant >> "$out"
        done
    done
done
echo "wrote $out"

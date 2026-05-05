#!/usr/bin/env python3
from __future__ import annotations

import csv
import pathlib
import sys
from collections import defaultdict

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = pathlib.Path(__file__).resolve().parent.parent
CSV_PATH = ROOT / "results" / "orderbook_add.csv"
OUT_PATH = ROOT / "docs" / "orderbook_perf.png"


def main() -> int:
    if not CSV_PATH.exists():
        print(f"missing {CSV_PATH}; run bench/run_all.sh first", file=sys.stderr)
        return 1

    by_n: dict[int, list[int]] = defaultdict(list)
    with CSV_PATH.open() as f:
        for row in csv.DictReader(f):
            by_n[int(row["orders"])].append(int(row["total_ns"]))

    sizes = sorted(by_n)
    means_s = [sum(by_n[n]) / len(by_n[n]) / 1e9 for n in sizes]

    plt.figure()
    plt.plot(sizes, means_s, marker="o", linestyle="-", color="b")
    plt.xlabel("Number of Orders")
    plt.ylabel("Execution Time (seconds)")
    plt.title("HFT Order Book add() throughput")
    plt.grid()
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(OUT_PATH, dpi=120, bbox_inches="tight")
    print(f"wrote {OUT_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

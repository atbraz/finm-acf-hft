#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["matplotlib"]
# ///
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

    by_label: dict[str, dict[int, list[int]]] = defaultdict(lambda: defaultdict(list))
    with CSV_PATH.open() as f:
        for row in csv.DictReader(f):
            by_label[row["label"]][int(row["orders"])].append(int(row["total_ns"]))

    plt.figure()
    for label in sorted(by_label):
        sizes = sorted(by_label[label])
        means_s = [sum(by_label[label][n]) / len(by_label[label][n]) / 1e9 for n in sizes]
        plt.plot(sizes, means_s, marker="o", linestyle="-", label=label)

    plt.xlabel("Number of Orders")
    plt.ylabel("Execution Time (seconds)")
    plt.title("HFT Order Book add() throughput")
    plt.grid()
    plt.legend()
    OUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(OUT_PATH, dpi=120, bbox_inches="tight")
    print(f"wrote {OUT_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

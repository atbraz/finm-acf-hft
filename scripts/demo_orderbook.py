#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["matplotlib"]
# ///
"""Live-updating bench demo.

Runs ./build/hft_bench across a sweep of N for both variants and draws each
point on the chart as the result lands. Used for the Phase 5 video beat.
"""
from __future__ import annotations

import pathlib
import subprocess
import sys
import time

import matplotlib.pyplot as plt

ROOT = pathlib.Path(__file__).resolve().parent.parent
BENCH = ROOT / "build" / "hft_bench"

SIZES = [1_000, 5_000, 10_000, 50_000, 100_000, 500_000, 1_000_000]
SEED = 42
PAUSE = 0.4


def run_bench(n: int, variant: str) -> tuple[int, float]:
    args = [str(BENCH), str(n), str(SEED)]
    if variant == "reserved":
        args.append("reserve")
    r = subprocess.run(args, capture_output=True, text=True, check=True)
    parts = r.stdout.strip().split(",")
    return int(parts[3]), float(parts[4])


def main() -> int:
    if not BENCH.exists():
        print(f"missing {BENCH}; run `just build` first", file=sys.stderr)
        return 1

    plt.ion()
    fig, ax = plt.subplots(figsize=(10, 6))
    ax.set_xlabel("Number of Orders")
    ax.set_ylabel("Execution Time (seconds)")
    ax.set_title("HFT Order Book add() throughput")
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.grid(True, which="both", linestyle="--", alpha=0.4)

    series = {
        "baseline": ax.plot([], [], marker="o", linestyle="-", color="C0", label="baseline")[0],
        "reserved": ax.plot([], [], marker="o", linestyle="-", color="C1", label="reserved")[0],
    }
    ax.legend(loc="upper left")

    status = ax.text(
        0.98, 0.05, "", transform=ax.transAxes,
        fontsize=11, ha="right", va="bottom",
        family="monospace",
        bbox=dict(boxstyle="round,pad=0.4", facecolor="white", edgecolor="0.6"),
    )

    data: dict[str, tuple[list[int], list[float]]] = {"baseline": ([], []), "reserved": ([], [])}

    for n in SIZES:
        for variant in ("baseline", "reserved"):
            status.set_text(f"running {variant:<8} N={n:>9,}")
            fig.canvas.draw_idle()
            plt.pause(0.05)

            t0 = time.perf_counter()
            total_ns, ns_per_op = run_bench(n, variant)
            wall_s = time.perf_counter() - t0

            xs, ys = data[variant]
            xs.append(n)
            ys.append(total_ns / 1e9)
            series[variant].set_data(xs, ys)
            ax.relim()
            ax.autoscale_view()

            status.set_text(
                f"{variant:<8} N={n:>9,}  {ns_per_op:>6.1f} ns/op  wall={wall_s*1000:>5.1f} ms"
            )
            fig.canvas.draw_idle()
            plt.pause(PAUSE)

    status.set_text("done. close window to exit.")
    plt.ioff()
    plt.show()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["matplotlib"]
# ///
"""Live-updating bench demo.

Starts at N=START_N and multiplies by MULTIPLIER each round. For every
round, runs the bench under both variants and plots both points together,
so baseline and reserved curves grow side by side. Loops until the user
closes the matplotlib window or hits Ctrl+C.
"""
from __future__ import annotations

import pathlib
import subprocess
import sys
import time

import matplotlib.pyplot as plt

ROOT = pathlib.Path(__file__).resolve().parent.parent
BENCH = ROOT / "build" / "hft_bench"

START_N = 1_000
MULTIPLIER = 2
SEED = 42
PAUSE_BETWEEN_ROUNDS = 0.6


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
    ax.set_title("HFT Order Book add() throughput  (close window to quit)")
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

    running = [True]
    fig.canvas.mpl_connect("close_event", lambda _e: running.__setitem__(0, False))

    n = START_N
    try:
        while running[0]:
            results: dict[str, tuple[int, float, float]] = {}
            for variant in ("baseline", "reserved"):
                if not running[0]:
                    break
                status.set_text(f"running {variant:<8} N={n:>11,}")
                fig.canvas.draw_idle()
                plt.pause(0.05)

                t0 = time.perf_counter()
                total_ns, ns_per_op = run_bench(n, variant)
                wall_s = time.perf_counter() - t0
                results[variant] = (total_ns, ns_per_op, wall_s)

            if not running[0] or len(results) < 2:
                break

            for variant, (total_ns, _, _) in results.items():
                xs, ys = data[variant]
                xs.append(n)
                ys.append(total_ns / 1e9)
                series[variant].set_data(xs, ys)
            ax.relim()
            ax.autoscale_view()

            base_ns_op = results["baseline"][1]
            resv_ns_op = results["reserved"][1]
            speedup = base_ns_op / resv_ns_op if resv_ns_op > 0 else 0.0
            status.set_text(
                f"N={n:>11,}  baseline={base_ns_op:>6.1f}  "
                f"reserved={resv_ns_op:>6.1f}  speedup={speedup:.2f}x"
            )
            fig.canvas.draw_idle()
            plt.pause(PAUSE_BETWEEN_ROUNDS)

            n = max(n + 1, int(n * MULTIPLIER))
    except KeyboardInterrupt:
        pass

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

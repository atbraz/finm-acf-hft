#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["matplotlib"]
# ///
"""Reads book depth snapshots from stdin and animates a depth chart.

Pipe `./build/hft_stream` into this script. Each FRAME...END block from
the C++ side is rendered as a two-sided horizontal bar chart: bids on
the left (negative quantity), asks on the right (positive quantity),
prices on the y-axis with the highest at the top.
"""
from __future__ import annotations

import os
import sys
from typing import Iterator

import matplotlib.pyplot as plt

MAX_LEVELS = 24


def read_frames(stream) -> Iterator[tuple[int, list[tuple[float, int, int]]]]:
    rows: list[tuple[float, int, int]] = []
    n_live = 0
    for line in stream:
        line = line.rstrip()
        if not line:
            continue
        if line.startswith("FRAME"):
            rows = []
            parts = line.split()
            n_live = int(parts[1]) if len(parts) > 1 else 0
        elif line == "END":
            yield n_live, rows
            rows = []
        elif line.startswith("L "):
            _, price, bid, ask = line.split()
            rows.append((float(price), int(bid), int(ask)))


def crop_around_mid(rows: list[tuple[float, int, int]]) -> list[tuple[float, int, int]]:
    if len(rows) <= MAX_LEVELS:
        return sorted(rows, key=lambda r: r[0], reverse=True)
    best_bid = max((p for p, b, _ in rows if b > 0), default=None)
    best_ask = min((p for p, _, a in rows if a > 0), default=None)
    if best_bid is None or best_ask is None:
        return sorted(rows, key=lambda r: r[0], reverse=True)[:MAX_LEVELS]
    mid = (best_bid + best_ask) / 2.0
    near = sorted(rows, key=lambda r: abs(r[0] - mid))[:MAX_LEVELS]
    return sorted(near, key=lambda r: r[0], reverse=True)


def main() -> int:
    plt.ion()
    fig, ax = plt.subplots(figsize=(11, 7))
    fig.canvas.mpl_connect("close_event", lambda _e: os._exit(0))

    for n_live, frame in read_frames(sys.stdin):
        cropped = crop_around_mid(frame)
        if not cropped:
            continue

        prices = [p for p, _, _ in cropped]
        bids = [-b for _, b, _ in cropped]
        asks = [a for _, _, a in cropped]

        ypos = list(range(len(cropped)))

        ax.cla()
        ax.barh(ypos, bids, color="#1f77b4", edgecolor="white", label="bids")
        ax.barh(ypos, asks, color="#d62728", edgecolor="white", label="asks")
        ax.axvline(0, color="black", linewidth=0.8)
        ax.set_yticks(ypos)
        ax.set_yticklabels([f"{p:.2f}" for p in prices])
        ax.invert_yaxis()
        ax.set_xlabel("Quantity")
        ax.set_ylabel("Price")
        ax.set_title(f"Order Book Depth   live orders: {n_live:,}   levels shown: {len(cropped)}")
        ax.grid(True, axis="x", linestyle="--", alpha=0.4)
        ax.legend(loc="upper right")

        x_max = max(max(asks, default=0), max((-b for b in bids), default=0))
        if x_max > 0:
            ax.set_xlim(-x_max * 1.1, x_max * 1.1)

        plt.pause(0.001)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Run bench at O0/O1/O2/O3 and print a combined comparison table."""

import subprocess
import sys

LEVELS = ["O0", "O1", "O2", "O3"]
DIRS = {"O0": "build", "O1": "build-O1", "O2": "build-O2", "O3": "build-O3"}


def run_bench(level: str) -> dict[tuple[str, int], float]:
    binary = f"{DIRS[level]}/bench"
    try:
        out = subprocess.check_output([binary], text=True)
    except FileNotFoundError:
        print(f"error: {binary} not found — run 'just build-all' first", file=sys.stderr)
        sys.exit(1)

    results: dict[tuple[str, int], float] = {}
    for line in out.splitlines():
        parts = line.split()
        # data rows have a numeric second-to-last field (stddev) and third-to-last (mean)
        if len(parts) >= 3 and parts[-1].replace(".", "").isdigit():
            try:
                n = int(parts[-3])
                mean = float(parts[-2])
                name = " ".join(parts[:-3])
                results[(name, n)] = mean
            except ValueError:
                continue
    return results


def main() -> None:
    data: dict[str, dict[tuple[str, int], float]] = {}
    for level in LEVELS:
        data[level] = run_bench(level)

    # collect all (name, n) keys in stable order
    keys: list[tuple[str, int]] = []
    seen: set[tuple[str, int]] = set()
    for level in LEVELS:
        for key in data[level]:
            if key not in seen:
                keys.append(key)
                seen.add(key)
    keys.sort(key=lambda k: (k[1], k[0]))  # sort by n, then name

    col_name = 36
    col_n = 6
    col_v = 12

    header = (
        f"{'function':<{col_name}}"
        f"{'n':>{col_n}}"
        + "".join(f"{lvl:>{col_v}}" for lvl in LEVELS)
        + f"{'best':>{col_n}}"
    )
    sep = "-" * len(header)

    print(header)
    print(sep)

    prev_n = None
    for name, n in keys:
        if prev_n is not None and n != prev_n:
            print(sep)
        prev_n = n

        vals = [data[lvl].get((name, n)) for lvl in LEVELS]
        best_idx = min(
            (idx for idx, v in enumerate(vals) if v is not None),
            key=lambda idx: vals[idx],  # type: ignore[index]
            default=None,
        )

        row = f"{name:<{col_name}}{n:>{col_n}}"
        for v in vals:
            cell = f"{v:.4f}" if v is not None else "n/a"
            row += f"{cell:>{col_v}}"
        row += f"{LEVELS[best_idx] if best_idx is not None else 'n/a':>{col_n}}"
        print(row)

    print(sep)
    print(f"\nValues are mean latency in ms (lower is better). 'best' = fastest opt level.")


if __name__ == "__main__":
    main()

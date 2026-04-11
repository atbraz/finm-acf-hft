# /// script
# requires-python = ">=3.12"
# dependencies = ["matplotlib"]
# ///

"""Live plot of HFT client prices and momentum entry points."""

import csv
import time
from pathlib import Path

import matplotlib.pyplot as plt

LOG = Path("build/prices.log")


def main():
    while not LOG.exists():
        print("Waiting for build/prices.log ...")
        time.sleep(1)

    fig, ax = plt.subplots()
    plt.ion()

    ids: list[int] = []
    prices: list[float] = []
    up_ids: list[int] = []
    up_prices: list[float] = []
    dn_ids: list[int] = []
    dn_prices: list[float] = []

    last_pos = 0

    try:
        while True:
            if LOG.exists() and LOG.stat().st_size < last_pos:
                last_pos = 0
                ids.clear(); prices.clear()
                up_ids.clear(); up_prices.clear()
                dn_ids.clear(); dn_prices.clear()

            with open(LOG) as f:
                f.seek(last_pos)
                reader = csv.DictReader(f) if last_pos == 0 else csv.DictReader(
                    f, fieldnames=["price_id", "price", "action"]
                )
                for row in reader:
                    pid = int(row["price_id"])
                    px = float(row["price"])
                    act = row["action"]
                    ids.append(pid)
                    prices.append(px)
                    if act == "order_up":
                        up_ids.append(pid)
                        up_prices.append(px)
                    elif act == "order_down":
                        dn_ids.append(pid)
                        dn_prices.append(px)
                last_pos = f.tell()

            ax.clear()
            ax.plot(ids, prices, color="steelblue", linewidth=1.2, label="Price")
            if up_ids:
                ax.scatter(up_ids, up_prices, marker="^", color="green", s=80,
                           zorder=5, label="Long entry")
            if dn_ids:
                ax.scatter(dn_ids, dn_prices, marker="v", color="red", s=80,
                           zorder=5, label="Short entry")
            ax.set_xlabel("Tick")
            ax.set_ylabel("Price")
            ax.set_title("HFT Momentum Strategy (Live)")
            ax.legend(loc="upper left")
            fig.tight_layout()

            plt.pause(0.5)
    except KeyboardInterrupt:
        pass
    finally:
        plt.ioff()
        plt.close()


if __name__ == "__main__":
    main()

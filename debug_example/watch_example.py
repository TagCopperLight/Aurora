#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["aurora-client"]
#
# [tool.uv.sources]
# aurora-client = { path = "../clients/python", editable = true }
# ///
"""
Aurora Debug - Variable watcher example.

Streams live variable updates to the debug example as watches.
Run with: uv run debug_example/watch_example.py
"""

import math
import time
from dataclasses import dataclass

from aurora_client import AuroraClient, AuroraConnectionError

INTERVAL = 0.1  # seconds between updates


@dataclass
class FakeProgram:
    """Variables that change over time, standing in for a real program's state."""

    t: float = 0.0
    counter: int = 0

    def advance(self, dt: float) -> None:
        self.t += dt
        self.counter += 1

    def watches(self) -> dict[str, object]:
        return {
            "velocity": round(math.sin(self.t) * 100.0, 2),       # oscillates -100 .. +100
            "position": round(self.t * 10.0 % 500.0, 2),          # ramps and wraps
            "health": max(0, 100 - self.counter % 101),           # counts down 100 -> 0
            "fps": round(60.0 + math.cos(self.t * 2) * 5, 1),     # 55 .. 65
        }


def main() -> None:
    program = FakeProgram()
    with AuroraClient() as aurora:
        print(f"Connected to {aurora.address}  (Ctrl+C to stop)")

        while True:
            watches = program.watches()
            aurora.watches(watches)
            print("  ".join(f"{name}={value:>8}" for name, value in watches.items()), end="\r")

            program.advance(INTERVAL)
            time.sleep(INTERVAL)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nStopped.")
    except AuroraConnectionError as error:
        print(error)

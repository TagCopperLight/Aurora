#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = ["aurora-client"]
#
# [tool.uv.sources]
# aurora-client = { path = "../clients/python", editable = true }
# ///
"""
Aurora Debug - TCP connection test script.

Sends one message of each type to the debug example, a few seconds apart.
Run with: uv run debug_example/test_connection.py
"""

import time

from aurora_client import AuroraClient, AuroraConnectionError, Custom, Log, LogLevel, Message, Profiling, Watch

MESSAGES: list[Message] = [
    Log("Aurora debug client connected"),
    Log("velocity is NaN", LogLevel.WARN),
    Watch("fps", "60.0"),
    Watch("memory", "128 MB"),
    Profiling({"frame_ms": 16.6, "cpu_ms": 2.1, "gpu_ms": 14.5}),
    Custom({"widget": "health_bar", "value": 0.75}),
    Log("texture load failed: missing.png", LogLevel.ERROR),
]


def main(delay: float = 2) -> None:
    with AuroraClient() as aurora:
        print(f"Connected to {aurora.address}")

        for message in MESSAGES:
            aurora.send(message)
            print(f"  -> [{message.type.name}] {message.encode()[1:].decode().rstrip()}")
            time.sleep(delay)

        print("All messages sent. Closing connection.")


if __name__ == "__main__":
    try:
        main()
    except AuroraConnectionError as error:
        print(error)

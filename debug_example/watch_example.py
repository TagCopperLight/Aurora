#!/usr/bin/env python3
"""
Aurora Debug - Variable watcher example.

Connects to the AuroraDebugServer and streams live variable updates.
Protocol: <1-byte MessageType> + <JSON payload> + '\n'
  MessageType 1 = Watch  ->  {"name": "<var>", "value": "<val>"}
"""

import socket
import json
import time
import math
import struct

HOST = "127.0.0.1"
PORT = 9000
INTERVAL = 0.1  # seconds between updates


def encode_watch(name: str, value) -> bytes:
    payload = json.dumps({"name": name, "value": str(value)}, separators=(",", ":"))
    return struct.pack("B", 1) + payload.encode() + b"\n"


def main() -> None:
    with socket.create_connection((HOST, PORT), timeout=5) as sock:
        print(f"Connected to {HOST}:{PORT}  (Ctrl+C to stop)")

        t = 0.0
        counter = 0

        while True:
            # --- Variables being watched ---
            velocity = round(math.sin(t) * 100.0, 2)        # oscillates -100 .. +100
            position = round(t * 10.0 % 500.0, 2)           # ramps and wraps
            health   = max(0, 100 - counter % 101)           # counts down 100 -> 0
            fps      = round(60.0 + math.cos(t * 2) * 5, 1) # 55 .. 65

            watches = [
                ("velocity", velocity),
                ("position", position),
                ("health",   health),
                ("fps",      fps),
            ]

            for name, val in watches:
                sock.sendall(encode_watch(name, val))

            print(
                f"  velocity={velocity:>8}  position={position:>8}"
                f"  health={health:>3}  fps={fps:>5}",
                end="\r",
            )

            t       += INTERVAL
            counter += 1
            time.sleep(INTERVAL)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nStopped.")
    except ConnectionRefusedError:
        print(f"Could not connect to {HOST}:{PORT} — is the debug app running?")

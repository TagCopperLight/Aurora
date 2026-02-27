#!/usr/bin/env python3
"""
Aurora Debug - TCP connection test script.

Protocol: each message is  <1-byte MessageType> + <JSON payload> + '\n'

MessageType values (aurora_debug_message.hpp):
    0 = Log
    1 = Watch
    2 = Profiling
    3 = Custom
"""

import socket
import json
import time
import struct

HOST = "127.0.0.1"
PORT = 9000


def encode(msg_type: int, payload: dict) -> bytes:
    """Encode a single debug message."""
    body = json.dumps(payload, separators=(",", ":"))
    return struct.pack("B", msg_type) + body.encode() + b"\n"


def send_all(messages: list[bytes], delay: float = 2) -> None:
    with socket.create_connection((HOST, PORT), timeout=5) as sock:
        print(f"Connected to {HOST}:{PORT}")

        for msg in messages:
            sock.sendall(msg)
            type_byte = msg[0]
            type_name = {0: "Log", 1: "Watch", 2: "Profiling", 3: "Custom"}.get(type_byte, "?")
            print(f"  -> [{type_name}] {msg[1:].decode().rstrip()}")
            time.sleep(delay)

        print("All messages sent. Closing connection.")


if __name__ == "__main__":
    messages = [
        encode(0, {"level": "info",  "msg": "Aurora debug client connected"}),
        encode(0, {"level": "warn",  "msg": "velocity is NaN"}),
        encode(1, {"name": "fps",    "value": "60.0"}),
        encode(1, {"name": "memory", "value": "128 MB"}),
        encode(2, {"frame_ms": 16.6, "cpu_ms": 2.1, "gpu_ms": 14.5}),
        encode(3, {"widget": "health_bar", "value": 0.75}),
        encode(0, {"level": "error", "msg": "texture load failed: missing.png"}),
    ]

    send_all(messages)

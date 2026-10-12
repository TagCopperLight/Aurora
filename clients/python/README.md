# aurora-client

Sends watches, logs and frames from Python to an Aurora app over TCP. It uses only
the standard library and needs Python 3.11 or newer. The wire format is described in
[`PROTOCOL.md`](../PROTOCOL.md).

## Install

In a uv project that has Aurora as a submodule:

```sh
uv add --editable ./Aurora/clients/python
```

A single script can declare it in its inline metadata instead, as the scripts in
`debug_example/` do, and then be run with `uv run script.py`:

```python
# /// script
# dependencies = ["aurora-client"]
#
# [tool.uv.sources]
# aurora-client = { path = "../clients/python", editable = true }
# ///
```

## Use

```python
from aurora_client import AuroraClient, AuroraUnavailableError, LogLevel

try:
    with AuroraClient("127.0.0.1", 9000) as aurora:
        aurora.log("Simulation started")
        aurora.watch("FPS", 60, section="STATS")
        aurora.watches({"TIME": "1.20 s", "BODIES": 5}, section="SIMULATION")
        aurora.frame({"scale": 220, "bodies": [0.1, 0.4, 0.05, 2]})
except AuroraUnavailableError as error:
    print(error)
```

`frame()` adds the `sent` timestamp that `AuroraFrameStream` uses to measure latency.

To send different kinds of messages in a single write, build them yourself and pass
them all to `send()`:

```python
from aurora_client import Custom, Log, Watch

aurora.send(
    Watch("TIME", "1.20 s", section="SIMULATION"),
    Log("Step took too long", LogLevel.WARN),
    Custom.frame({"scale": 220}),
)
```

`AuroraUnavailableError` and `AuroraDisconnectedError` both subclass
`AuroraConnectionError`, which subclasses the built-in `ConnectionError`.

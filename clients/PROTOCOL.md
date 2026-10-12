# Aurora debug protocol

Programs send data to an Aurora app over TCP. The app runs an `AuroraDebugServer`
(port 9000 by default) and each client opens one connection to it. Data only flows
from the client to the server; the server never replies.

The clients in this folder implement this protocol. Use one if it exists for your
language, and update this file first when the protocol changes.

## Framing

Each message is one line:

```
<1-byte message type><JSON object>\n
```

- The JSON object must fit on one line: newlines inside strings must be escaped,
  which every JSON encoder does by default.
- Non-ASCII characters should be sent as `\uXXXX` escapes. Aurora shows anything
  outside ASCII as `?`.
- Empty lines are ignored. Several messages can be sent in a single write.

## Message types

| Byte | Type      | Handled by                                      |
| ---- | --------- | ----------------------------------------------- |
| `0`  | Log       | Nothing built in; apps may log it               |
| `1`  | Watch     | `WatchMessage::parse`, shown in an `AuroraPanel` |
| `2`  | Profiling | Nothing built in; reserved for timing data       |
| `3`  | Custom    | The app, usually through `AuroraFrameStream`    |

### Log

```json
{"level": "info", "msg": "Simulation started"}
```

`level` is one of `info`, `warn` or `error`.

### Watch

```json
{"section": "SIMULATION", "name": "SIM TIME", "value": "12.40 s"}
```

- `name` is required; watches without it are ignored.
- `value` is shown as text, so format numbers before sending them.
- `section` is optional and defaults to `WATCHES`. Sections are created in the
  order their first watch arrives.

### Profiling

```json
{"frame_ms": 16.6, "cpu_ms": 2.1, "gpu_ms": 14.5}
```

An object of timings in milliseconds. No built-in component reads it yet.

### Custom

Application-defined data, typically one frame of a simulation:

```json
{"sent": 1760270400.25, "scale": 220, "bodies": [0.1, 0.4, 0.05, 2]}
```

- The keys are agreed between the sender and the app receiving them.
- Aurora's readers (`aurora_debug_json.hpp`) only handle flat objects: top-level
  strings, numbers, and arrays of numbers. Nested objects are not supported.
- `sent` is optional: the time the frame was sent, in Unix seconds.
  `AuroraFrameStream` uses it to show latency. Both programs must run on the same
  machine for this to be meaningful, since it compares their wall clocks.
- `AuroraFrameStream` only draws the latest frame. Frames that arrive between two
  renders are counted as not drawn.

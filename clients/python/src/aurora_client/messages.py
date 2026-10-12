"""Messages understood by AuroraDebugServer. The wire format is described in clients/PROTOCOL.md."""

from __future__ import annotations

import json
import time
from abc import ABC, abstractmethod
from collections.abc import Mapping
from dataclasses import dataclass, field
from enum import IntEnum, StrEnum
from typing import Any, ClassVar


class MessageType(IntEnum):
    LOG = 0
    WATCH = 1
    PROFILING = 2
    CUSTOM = 3


class LogLevel(StrEnum):
    INFO = "info"
    WARN = "warn"
    ERROR = "error"


class Message(ABC):
    """One line of the protocol: a type byte, a JSON object, then a newline."""

    type: ClassVar[MessageType]

    @abstractmethod
    def payload(self) -> dict[str, Any]:
        """The JSON object sent after the type byte."""

    def encode(self) -> bytes:
        body = json.dumps(self.payload(), separators=(",", ":"))
        return bytes([self.type]) + body.encode() + b"\n"


@dataclass(frozen=True, slots=True)
class Log(Message):
    type: ClassVar[MessageType] = MessageType.LOG

    text: str
    level: LogLevel = LogLevel.INFO

    def payload(self) -> dict[str, Any]:
        return {"level": str(self.level), "msg": self.text}


@dataclass(frozen=True, slots=True)
class Watch(Message):
    """A labelled value shown in a panel section. The value is sent as text."""

    type: ClassVar[MessageType] = MessageType.WATCH

    name: str
    value: object
    section: str | None = None  # Aurora defaults to "WATCHES"

    def payload(self) -> dict[str, Any]:
        payload = {"name": self.name, "value": str(self.value)}
        if self.section is not None:
            payload["section"] = self.section
        return payload


@dataclass(frozen=True, slots=True)
class Profiling(Message):
    """Timings in milliseconds, e.g. {"frame_ms": 16.6}."""

    type: ClassVar[MessageType] = MessageType.PROFILING

    timings: Mapping[str, float]

    def payload(self) -> dict[str, Any]:
        return dict(self.timings)


@dataclass(frozen=True, slots=True)
class Custom(Message):
    """Application-defined data, read by the receiving app. Aurora only reads flat objects."""

    type: ClassVar[MessageType] = MessageType.CUSTOM

    data: Mapping[str, Any]
    sent: float | None = None  # Unix seconds, used by AuroraFrameStream to show latency

    @classmethod
    def frame(cls, data: Mapping[str, Any]) -> Custom:
        """A frame stamped with the current time, so the receiver can measure latency."""
        return cls(data, sent=time.time())

    def payload(self) -> dict[str, Any]:
        payload = dict(self.data)
        if self.sent is not None:
            payload["sent"] = self.sent
        return payload

from __future__ import annotations

import select
import socket
from collections.abc import Mapping
from types import TracebackType
from typing import Any, Self

from .messages import Command, Custom, Log, LogLevel, Message, Watch

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 9000


class AuroraConnectionError(ConnectionError):
    """Base class for errors talking to an Aurora app."""


class AuroraUnavailableError(AuroraConnectionError):
    """No Aurora app is listening at the given address."""


class AuroraDisconnectedError(AuroraConnectionError):
    """The Aurora app closed the connection."""


class AuroraClient:
    """A connection to an Aurora app running AuroraDebugServer.

    Use it as a context manager so the connection is always closed:

        with AuroraClient() as aurora:
            aurora.watch("FPS", 60, section="STATS")
    """

    def __init__(self, host: str = DEFAULT_HOST, port: int = DEFAULT_PORT, timeout: float = 5.0) -> None:
        self.host = host
        self.port = port
        self.timeout = timeout
        self._socket: socket.socket | None = None
        self._received = b""  # bytes read from the app that do not form a complete line yet

    def __repr__(self) -> str:
        state = "connected" if self.connected else "closed"
        return f"{type(self).__name__}({self.address!r}, {state})"

    def __enter__(self) -> Self:
        return self.connect()

    def __exit__(self, exc_type: type[BaseException] | None, exc: BaseException | None, tb: TracebackType | None) -> None:
        self.close()

    @property
    def address(self) -> str:
        return f"{self.host}:{self.port}"

    @property
    def connected(self) -> bool:
        return self._socket is not None

    def connect(self) -> Self:
        if self._socket is None:
            try:
                self._socket = socket.create_connection((self.host, self.port), timeout=self.timeout)
                self._socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            except (ConnectionRefusedError, TimeoutError) as error:
                raise AuroraUnavailableError(f"No Aurora app is listening on {self.address}") from error
        return self

    def close(self) -> None:
        if self._socket is not None:
            self._socket.close()
            self._socket = None
            self._received = b""

    def send(self, *messages: Message) -> None:
        """Sends all messages in a single write."""
        if self._socket is None:
            raise AuroraConnectionError("Not connected, call connect() first")
        try:
            self._socket.sendall(b"".join(message.encode() for message in messages))
        except (BrokenPipeError, ConnectionResetError) as error:
            self.close()
            raise AuroraDisconnectedError(f"Aurora app at {self.address} closed the connection") from error

    def commands(self) -> list[Command]:
        """Returns the commands the app sent since the last call, oldest first. Never blocks."""
        if self._socket is None:
            raise AuroraConnectionError("Not connected, call connect() first")
        while select.select([self._socket], [], [], 0)[0]:
            try:
                data = self._socket.recv(4096)
            except ConnectionResetError:
                data = b""
            if not data:
                self.close()
                raise AuroraDisconnectedError(f"Aurora app at {self.address} closed the connection")
            self._received += data

        *lines, self._received = self._received.split(b"\n")
        return [command for line in lines if (command := Command.decode(line)) is not None]

    def log(self, text: str, level: LogLevel = LogLevel.INFO) -> None:
        self.send(Log(text, level))

    def watch(self, name: str, value: object, section: str | None = None) -> None:
        self.send(Watch(name, value, section))

    def watches(self, values: Mapping[str, object], section: str | None = None) -> None:
        """Sends several watches at once. They appear in the section in the mapping's order."""
        self.send(*(Watch(name, value, section) for name, value in values.items()))

    def frame(self, data: Mapping[str, Any]) -> None:
        """Sends a frame stamped with the current time, for apps using AuroraFrameStream."""
        self.send(Custom.frame(data))

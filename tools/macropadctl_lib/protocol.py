from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable

import hid

RAW_PACKET_SIZE = 32
DEFAULT_VID = 0x32AC
DEFAULT_PID = 0x0013
DEFAULT_USAGE_PAGE = 0xFF60
DEFAULT_USAGE = 0x61

CMD_GET_INFO = 0x40
CMD_GET_COLOR = 0x41
CMD_SET_COLOR = 0x42
CMD_GET_KEYCODE = 0x43
CMD_SET_KEYCODE = 0x44
CMD_COMMIT = 0x45
CMD_RELOAD = 0x46
CMD_RESET_TO_DEFAULTS = 0x47


@dataclass(frozen=True)
class DeviceInfo:
    version: int
    num_modes: int
    led_count: int
    matrix_rows: int
    matrix_cols: int


class MacropadProtocol:
    def __init__(
        self,
        *,
        vid: int = DEFAULT_VID,
        pid: int = DEFAULT_PID,
        usage_page: int = DEFAULT_USAGE_PAGE,
        usage: int = DEFAULT_USAGE,
        timeout_ms: int = 2000,
    ) -> None:
        self._timeout_ms = timeout_ms
        self._device = hid.Device(path=self._select_path(vid=vid, pid=pid, usage_page=usage_page, usage=usage))

    def __enter__(self) -> "MacropadProtocol":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def close(self) -> None:
        self._device.close()

    @staticmethod
    def _select_path(*, vid: int, pid: int, usage_page: int, usage: int) -> bytes:
        matches = [
            device
            for device in hid.enumerate(vid, pid)
            if device.get("usage_page") == usage_page and device.get("usage") == usage
        ]
        if not matches:
            raise RuntimeError("No matching raw-HID device found")
        if len(matches) > 1:
            raise RuntimeError("Multiple matching raw-HID devices found; disconnect extras or extend selector logic")
        return matches[0]["path"]

    def _transceive(self, command_id: int, payload: Iterable[int] = ()) -> bytes:
        packet = bytearray(RAW_PACKET_SIZE)
        packet[0] = command_id
        for index, value in enumerate(payload, start=1):
            packet[index] = value

        written = self._device.write(bytes([0]) + bytes(packet))
        if written != RAW_PACKET_SIZE + 1:
            raise RuntimeError(f"Short HID write: expected {RAW_PACKET_SIZE + 1}, got {written}")

        response = bytes(self._device.read(RAW_PACKET_SIZE, self._timeout_ms))
        if len(response) != RAW_PACKET_SIZE:
            raise RuntimeError("Timed out waiting for raw-HID response")
        if response[0] != command_id:
            raise RuntimeError(f"Unexpected response command 0x{response[0]:02X} for request 0x{command_id:02X}")
        return response

    def get_info(self) -> DeviceInfo:
        response = self._transceive(CMD_GET_INFO)
        return DeviceInfo(
            version=int.from_bytes(response[1:3], "little"),
            num_modes=int.from_bytes(response[3:5], "little"),
            led_count=int.from_bytes(response[5:7], "little"),
            matrix_rows=response[7],
            matrix_cols=response[8],
        )

    def get_color(self, *, mode: int, led: int) -> tuple[int, int, int]:
        response = self._transceive(CMD_GET_COLOR, [mode, led])
        return response[1], response[2], response[3]

    def set_color(self, *, mode: int, led: int, rgb: tuple[int, int, int]) -> None:
        self._transceive(CMD_SET_COLOR, [mode, led, *rgb])

    def get_keycode(self, *, mode: int, row: int, col: int) -> int:
        response = self._transceive(CMD_GET_KEYCODE, [mode, row, col])
        return int.from_bytes(response[1:3], "little")

    def set_keycode(self, *, mode: int, row: int, col: int, keycode: int) -> None:
        self._transceive(CMD_SET_KEYCODE, [mode, row, col, keycode & 0xFF, (keycode >> 8) & 0xFF])

    def commit(self) -> None:
        self._transceive(CMD_COMMIT)

    def reload(self) -> None:
        self._transceive(CMD_RELOAD)

    def reset_to_defaults(self) -> None:
        self._transceive(CMD_RESET_TO_DEFAULTS)

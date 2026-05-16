from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any

import yaml

ROOT = Path(__file__).resolve().parents[2]
PADS_DIR = ROOT / "pads"


@dataclass(frozen=True)
class PadPosition:
    label: str
    row: int
    col: int
    led: int
    physical_row: int
    physical_col: int
    aliases: tuple[str, ...] = ()


class PadProfile:
    def __init__(self, data: dict[str, Any]) -> None:
        self.name = data["name"]
        self.led_count = int(data["led_count"])
        self.matrix_rows = int(data["matrix_rows"])
        self.matrix_cols = int(data["matrix_cols"])
        self.palette_aliases = {
            ("off" if key is False else str(key)): tuple(int(channel) for channel in value)
            for key, value in (data.get("palette_aliases") or {}).items()
        }
        self.positions = [
            PadPosition(
                label=entry["label"],
                row=int(entry["row"]),
                col=int(entry["col"]),
                led=int(entry["led"]),
                physical_row=int(entry["physical_row"]),
                physical_col=int(entry["physical_col"]),
                aliases=tuple(entry.get("aliases") or ()),
            )
            for entry in data.get("positions", [])
        ]

        self._labels: dict[str, PadPosition] = {}
        for position in self.positions:
            for name in (position.label, *position.aliases):
                self._labels[name] = position

        self._by_led = {position.led: position for position in self.positions}
        self._by_matrix = {(position.row, position.col): position for position in self.positions}

    def dimensions_match(self, *, led_count: int, matrix_rows: int, matrix_cols: int) -> bool:
        return (
            self.led_count == led_count
            and self.matrix_rows == matrix_rows
            and self.matrix_cols == matrix_cols
        )

    def resolve_label(self, label: str) -> PadPosition:
        if label in self._labels:
            return self._labels[label]
        raise KeyError(f"Unknown pad position label: {label}")

    def position_for_led(self, led: int) -> PadPosition | None:
        return self._by_led.get(led)

    def position_for_matrix(self, row: int, col: int) -> PadPosition | None:
        return self._by_matrix.get((row, col))

    def display_name_for_led(self, led: int) -> str:
        position = self.position_for_led(led)
        return position.label if position else f"led:{led}"

    def display_name_for_matrix(self, row: int, col: int) -> str:
        position = self.position_for_matrix(row, col)
        return position.label if position else f"r{row}c{col}"

    def color_for_name(self, name: str) -> tuple[int, int, int]:
        color = self.palette_aliases.get(name)
        if color is None:
            raise KeyError(f"Unknown colour alias: {name}")
        return color

    def color_name_for_rgb(self, rgb: tuple[int, int, int]) -> str | list[int]:
        for name, candidate in self.palette_aliases.items():
            if candidate == rgb:
                return name
        return list(rgb)



def load_pad_profile(name: str) -> PadProfile:
    path = PADS_DIR / f"{name}.yaml"
    if not path.exists():
        raise FileNotFoundError(f"Pad profile not found: {path}")
    return PadProfile(yaml.safe_load(path.read_text()))



def iter_pad_profiles() -> list[PadProfile]:
    return [PadProfile(yaml.safe_load(path.read_text())) for path in sorted(PADS_DIR.glob("*.yaml"))]



def detect_pad_profile(*, led_count: int, matrix_rows: int, matrix_cols: int) -> PadProfile:
    matches = [
        profile
        for profile in iter_pad_profiles()
        if profile.dimensions_match(
            led_count=led_count,
            matrix_rows=matrix_rows,
            matrix_cols=matrix_cols,
        )
    ]
    if not matches:
        raise LookupError("No pad profile matches the connected device dimensions")
    if len(matches) > 1:
        names = ", ".join(profile.name for profile in matches)
        raise LookupError(f"Multiple pad profiles match the connected device: {names}")
    return matches[0]

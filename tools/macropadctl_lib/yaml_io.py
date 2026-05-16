from __future__ import annotations

from copy import deepcopy
from pathlib import Path
from typing import Any

import yaml

from .keycode_names import format_keycode, parse_keycode
from .pads import PadProfile



def load_user_config(path: str | Path) -> dict[str, Any]:
    with Path(path).open() as handle:
        return yaml.safe_load(handle)



def parse_rgb_value(value: Any, profile: PadProfile) -> tuple[int, int, int]:
    if value is False:
        return profile.color_for_name("off")

    if isinstance(value, str):
        if value in profile.palette_aliases:
            return profile.color_for_name(value)
        parts = [part.strip() for part in value.split(",")]
        if len(parts) == 3:
            return tuple(int(part) for part in parts)
        raise ValueError(f"Unsupported colour string: {value}")

    if isinstance(value, (list, tuple)) and len(value) == 3:
        return tuple(int(channel) for channel in value)

    raise ValueError(f"Unsupported colour value: {value!r}")



def dump_device_state(*, profile: PadProfile, device_info: Any, state: dict[str, Any]) -> str:
    modes: dict[int, dict[str, Any]] = {}

    for mode in range(device_info.num_modes):
        colors = {
            profile.display_name_for_led(led): profile.color_name_for_rgb(tuple(state["colors"][mode][led]))
            for led in range(device_info.led_count)
        }
        keymap = {
            profile.display_name_for_matrix(row, col): format_keycode(state["keymap"][mode][row][col])
            for row in range(device_info.matrix_rows)
            for col in range(device_info.matrix_cols)
        }
        modes[mode] = {"colors": colors, "keymap": keymap}

    document = {
        "pad": profile.name,
        "modes": modes,
    }
    return yaml.safe_dump(document, sort_keys=False)



def apply_user_config(*, config: dict[str, Any], profile: PadProfile, state: dict[str, Any], device_info: Any) -> dict[str, Any]:
    next_state = deepcopy(state)
    modes = config.get("modes") or {}

    for mode_key, mode_spec in modes.items():
        mode = int(mode_key)
        if mode < 0 or mode >= device_info.num_modes:
            raise ValueError(f"Mode {mode} is out of range for connected device")

        colors = mode_spec.get("colors") or {}
        rows = mode_spec.get("rows") or {}
        default_color = colors.get("default")
        if default_color is not None:
            rgb = parse_rgb_value(default_color, profile)
            for led in range(device_info.led_count):
                next_state["colors"][mode][led] = list(rgb)

        for row_key, color_value in rows.items():
            physical_row = int(row_key)
            rgb = parse_rgb_value(color_value, profile)
            for position in profile.positions:
                if position.physical_row == physical_row:
                    next_state["colors"][mode][position.led] = list(rgb)

        for label, color_value in colors.items():
            if label == "default":
                continue
            position = profile.resolve_label(label)
            next_state["colors"][mode][position.led] = list(parse_rgb_value(color_value, profile))

        for label, keycode_name in (mode_spec.get("keymap") or {}).items():
            position = profile.resolve_label(label)
            next_state["keymap"][mode][position.row][position.col] = parse_keycode(str(keycode_name))

    return next_state

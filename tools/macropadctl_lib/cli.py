from __future__ import annotations

import argparse
from pathlib import Path
from typing import Any

from .keycode_names import parse_keycode
from .pads import detect_pad_profile, load_pad_profile
from .protocol import DEFAULT_PID, DEFAULT_USAGE, DEFAULT_USAGE_PAGE, DEFAULT_VID, MacropadProtocol
from .yaml_io import apply_user_config, dump_device_state, load_user_config, parse_rgb_value



def build_common_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--vid", type=lambda value: int(value, 0), default=DEFAULT_VID)
    parser.add_argument("--pid", type=lambda value: int(value, 0), default=DEFAULT_PID)
    parser.add_argument("--usage-page", type=lambda value: int(value, 0), default=DEFAULT_USAGE_PAGE)
    parser.add_argument("--usage", type=lambda value: int(value, 0), default=DEFAULT_USAGE)
    parser.add_argument("--timeout-ms", type=int, default=2000)
    parser.add_argument("--pad", help="Pad profile name override for commands that resolve labels")
    return parser



def read_full_state(device: MacropadProtocol, info: Any) -> dict[str, Any]:
    colors = []
    keymap = []

    for mode in range(info.num_modes):
        mode_colors = []
        for led in range(info.led_count):
            mode_colors.append(list(device.get_color(mode=mode, led=led)))
        colors.append(mode_colors)

        mode_keymap = []
        for row in range(info.matrix_rows):
            row_keycodes = []
            for col in range(info.matrix_cols):
                row_keycodes.append(device.get_keycode(mode=mode, row=row, col=col))
            mode_keymap.append(row_keycodes)
        keymap.append(mode_keymap)

    return {"colors": colors, "keymap": keymap}



def resolve_profile(args: argparse.Namespace, info: Any, config_pad_name: str | None = None):
    if config_pad_name:
        profile = load_pad_profile(config_pad_name)
    elif args.pad:
        profile = load_pad_profile(args.pad)
    else:
        profile = detect_pad_profile(
            led_count=info.led_count,
            matrix_rows=info.matrix_rows,
            matrix_cols=info.matrix_cols,
        )

    if not profile.dimensions_match(
        led_count=info.led_count,
        matrix_rows=info.matrix_rows,
        matrix_cols=info.matrix_cols,
    ):
        raise ValueError(
            f"Pad profile {profile.name} dimensions do not match device "
            f"({profile.led_count}/{profile.matrix_rows}/{profile.matrix_cols} vs "
            f"{info.led_count}/{info.matrix_rows}/{info.matrix_cols})"
        )
    return profile



def print_info(info: Any, *, pad_name: str | None = None) -> None:
    print(f"version: {info.version}")
    print(f"num_modes: {info.num_modes}")
    print(f"led_count: {info.led_count}")
    print(f"matrix_rows: {info.matrix_rows}")
    print(f"matrix_cols: {info.matrix_cols}")
    if pad_name:
        print(f"pad: {pad_name}")



def apply_state(device: MacropadProtocol, current_state: dict[str, Any], next_state: dict[str, Any], info: Any, *, persist: bool) -> None:
    for mode in range(info.num_modes):
        for led in range(info.led_count):
            if current_state["colors"][mode][led] != next_state["colors"][mode][led]:
                device.set_color(mode=mode, led=led, rgb=tuple(next_state["colors"][mode][led]))

        for row in range(info.matrix_rows):
            for col in range(info.matrix_cols):
                if current_state["keymap"][mode][row][col] != next_state["keymap"][mode][row][col]:
                    device.set_keycode(mode=mode, row=row, col=col, keycode=next_state["keymap"][mode][row][col])

    if persist:
        device.commit()



def main(argv: list[str] | None = None) -> int:
    common = build_common_parser()
    parser = argparse.ArgumentParser(prog="macropadctl")
    subparsers = parser.add_subparsers(dest="command", required=True)

    subparsers.add_parser("info", parents=[common])
    subparsers.add_parser("dump", parents=[common])

    apply_parser = subparsers.add_parser("apply", parents=[common])
    apply_parser.add_argument("--persist", action="store_true")
    apply_parser.add_argument("config")

    set_color_parser = subparsers.add_parser("set-color", parents=[common])
    set_color_parser.add_argument("--persist", action="store_true")
    set_color_parser.add_argument("--mode", required=True, type=int)
    target_group = set_color_parser.add_mutually_exclusive_group(required=True)
    target_group.add_argument("--key")
    target_group.add_argument("--led", type=int)
    color_group = set_color_parser.add_mutually_exclusive_group(required=True)
    color_group.add_argument("--color")
    color_group.add_argument("--rgb")

    set_keycode_parser = subparsers.add_parser("set-keycode", parents=[common])
    set_keycode_parser.add_argument("--persist", action="store_true")
    set_keycode_parser.add_argument("--mode", required=True, type=int)
    key_target_group = set_keycode_parser.add_mutually_exclusive_group(required=True)
    key_target_group.add_argument("--key")
    key_target_group.add_argument("--matrix", help="Matrix coordinate as row,col")
    set_keycode_parser.add_argument("keycode")

    subparsers.add_parser("reload", parents=[common])
    subparsers.add_parser("reset", parents=[common])

    args = parser.parse_args(argv)

    with MacropadProtocol(
        vid=args.vid,
        pid=args.pid,
        usage_page=args.usage_page,
        usage=args.usage,
        timeout_ms=args.timeout_ms,
    ) as device:
        info = device.get_info()

        if args.command == "info":
            pad_name = None
            try:
                pad_name = resolve_profile(args, info).name if not args.pad else load_pad_profile(args.pad).name
            except Exception:
                pad_name = args.pad
            print_info(info, pad_name=pad_name)
            return 0

        if args.command == "reload":
            device.reload()
            return 0

        if args.command == "reset":
            device.reset_to_defaults()
            return 0

        if args.command == "dump":
            profile = resolve_profile(args, info)
            state = read_full_state(device, info)
            print(dump_device_state(profile=profile, device_info=info, state=state), end="")
            return 0

        if args.command == "apply":
            config = load_user_config(args.config)
            profile = resolve_profile(args, info, config_pad_name=config.get("pad"))
            current_state = read_full_state(device, info)
            next_state = apply_user_config(config=config, profile=profile, state=current_state, device_info=info)
            apply_state(device, current_state, next_state, info, persist=args.persist)
            return 0

        profile = resolve_profile(args, info)

        if args.command == "set-color":
            if args.mode < 0 or args.mode >= info.num_modes:
                raise ValueError(f"Mode {args.mode} is out of range")
            led = args.led if args.led is not None else profile.resolve_label(args.key).led
            rgb = parse_rgb_value(args.color if args.color is not None else args.rgb, profile)
            device.set_color(mode=args.mode, led=led, rgb=rgb)
            if args.persist:
                device.commit()
            return 0

        if args.command == "set-keycode":
            if args.mode < 0 or args.mode >= info.num_modes:
                raise ValueError(f"Mode {args.mode} is out of range")
            if args.key is not None:
                position = profile.resolve_label(args.key)
                row, col = position.row, position.col
            else:
                row, col = (int(part.strip()) for part in args.matrix.split(",", 1))
            device.set_keycode(mode=args.mode, row=row, col=col, keycode=parse_keycode(args.keycode))
            if args.persist:
                device.commit()
            return 0

    return 0

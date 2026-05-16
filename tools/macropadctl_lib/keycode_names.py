from __future__ import annotations

KEYCODE_NAME_TO_VALUE = {
    "KC_NO": 0x0000,
    "KC_TRNS": 0x0001,
    "KC_1": 0x001E,
    "KC_2": 0x001F,
    "KC_3": 0x0020,
    "KC_4": 0x0021,
    "KC_5": 0x0022,
    "KC_6": 0x0023,
    "KC_7": 0x0024,
    "KC_8": 0x0025,
    "KC_9": 0x0026,
    "KC_0": 0x0027,
    "KC_ENT": 0x0028,
    "KC_BSPC": 0x002A,
    "KC_HOME": 0x004A,
    "KC_PGUP": 0x004B,
    "KC_DEL": 0x004C,
    "KC_END": 0x004D,
    "KC_PGDN": 0x004E,
    "KC_PAST": 0x0055,
    "KC_PMNS": 0x0056,
    "KC_PPLS": 0x0057,
    "KC_F13": 0x0068,
    "KC_F14": 0x0069,
    "KC_F15": 0x006A,
    "KC_F16": 0x006B,
    "KC_F17": 0x006C,
    "KC_F18": 0x006D,
    "KC_F19": 0x006E,
    "KC_F20": 0x006F,
    "KC_F21": 0x0070,
    "KC_F22": 0x0071,
    "KC_F23": 0x0072,
    "KC_F24": 0x0073,
    "S(KC_NUBS)": 0x0264,
    "QK_BOOT": 0x7C00,
    "CYCLE_MODE": 0x7E40,
}

KEYCODE_VALUE_TO_NAME = {value: name for name, value in KEYCODE_NAME_TO_VALUE.items()}


def parse_keycode(name: str) -> int:
    normalized = name.strip()
    if normalized in KEYCODE_NAME_TO_VALUE:
        return KEYCODE_NAME_TO_VALUE[normalized]

    if normalized.lower().startswith("0x"):
        return int(normalized, 16)

    raise KeyError(f"Unsupported keycode: {name}")


def format_keycode(value: int) -> str:
    return KEYCODE_VALUE_TO_NAME.get(value, f"0x{value:04X}")

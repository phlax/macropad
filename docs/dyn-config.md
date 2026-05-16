# Dynamic configuration

This proof-of-concept adds an EEPROM-backed dynamic configuration blob for the QMK firmware plus a Python CLI that talks to the pad over raw HID. The protocol and EEPROM layout are pad-agnostic: the firmware only reports dimensions and stores raw indices, while human-friendly labels, palette aliases, and physical positions live in host-side pad profiles under `pads/`.

## Install

Create a virtual environment and install the host dependencies:

```bash
cd /home/runner/work/macropad/macropad
python3 -m venv /tmp/macropadctl-venv
source /tmp/macropadctl-venv/bin/activate
pip install -r requirements.txt
```

Run the CLI directly from the repo:

```bash
/home/runner/work/macropad/macropad/tools/macropadctl info
```

### Linux udev rule

If hidapi cannot open the raw-HID interface as a regular user, create `/etc/udev/rules.d/60-macropad.rules`:

```udev
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="32ac", ATTRS{idProduct}=="0013", MODE="0666"
```

Then reload rules and replug the pad.

## CLI commands

```bash
# show device info
/home/runner/work/macropad/macropad/tools/macropadctl info

# dump current device state to YAML
/home/runner/work/macropad/macropad/tools/macropadctl dump > my-config.yaml

# apply YAML live in RAM only
/home/runner/work/macropad/macropad/tools/macropadctl apply my-config.yaml

# apply YAML and persist it to EEPROM
/home/runner/work/macropad/macropad/tools/macropadctl apply --persist my-config.yaml

# set a single LED by logical key label or raw LED index
/home/runner/work/macropad/macropad/tools/macropadctl set-color --mode 0 --key Bksp --color blue
/home/runner/work/macropad/macropad/tools/macropadctl set-color --mode 0 --led 18 --rgb 255,0,0

# set a single keycode live
/home/runner/work/macropad/macropad/tools/macropadctl set-keycode --mode 0 --key Bksp KC_DEL

# discard RAM changes and reload EEPROM contents
/home/runner/work/macropad/macropad/tools/macropadctl reload

# wipe the EEPROM magic; next reboot reseeds from PROGMEM defaults
/home/runner/work/macropad/macropad/tools/macropadctl reset
```

All host commands accept optional raw-HID selector overrides: `--vid`, `--pid`, `--usage-page`, `--usage`, `--timeout-ms`, and `--pad`.

## YAML schema

User configs reference a pad profile and then describe one or more modes:

```yaml
pad: framework_macropad
modes:
  0:
    colors:
      MODE: magenta
      Bksp: red
      "+": yellow
      default: white
    keymap:
      MODE: CYCLE_MODE
      Bksp: KC_BSPC
      "+": KC_PPLS
  3:
    rows:
      0: red
      1: orange
      2: yellow
      3: green
      4: blue
      5: magenta
```

Rules:

- `pad:` names a file in `pads/<name>.yaml`.
- `colors:` accepts palette aliases from the pad profile or raw RGB triples.
- `colors.default` fills any LED in that mode that is not explicitly listed.
- `rows:` is a convenience helper keyed by `physical_row` from the pad profile.
- `keymap:` maps logical key labels to 16-bit QMK keycodes.
- The CLI validates the chosen pad profile against the dimensions returned by `GET_INFO`.

The shipped example `/home/runner/work/macropad/macropad/examples/phlax.yaml` recreates the current defaults.

## Pad profiles

Pad profiles describe host-side naming and geometry only. The firmware never hard-codes labels or layout metadata.

Current shipped profile:

- `/home/runner/work/macropad/macropad/pads/framework_macropad.yaml`

Each profile contains:

- `name`
- `led_count`
- `matrix_rows`
- `matrix_cols`
- `positions[]` entries with `label`, `row`, `col`, `led`, `physical_row`, `physical_col`, and optional `aliases`
- `palette_aliases`

To add a new pad, create another file in `pads/` with the same schema and use its `name` in user YAML. If more than one profile matches the connected device dimensions, pass `--pad` explicitly.

## Keycode table

The POC keeps a curated keycode table in `/home/runner/work/macropad/macropad/tools/macropadctl_lib/keycode_names.py`.

It currently covers:

- `KC_0..KC_9`
- `KC_PAST`, `KC_PPLS`, `KC_PMNS`, `KC_BSPC`, `KC_ENT`, `KC_DEL`
- `KC_HOME`, `KC_END`, `KC_PGUP`, `KC_PGDN`
- `KC_F13..KC_F24`
- `S(KC_NUBS)`
- `QK_BOOT`, `KC_NO`, `KC_TRNS`, `CYCLE_MODE`

To extend it, add entries to `KEYCODE_NAME_TO_VALUE`; unknown dump values are emitted as `0xNNNN` and can also be applied in that numeric form.

## Protocol reference

All packets are 32 bytes. Byte 0 is the command ID; unused bytes are zero. Responses echo the command ID in byte 0.

| ID | Name | Request payload | Response payload |
|---|---|---|---|
| `0x40` | `GET_INFO` | — | `u16 version`, `u16 num_modes`, `u16 led_count`, `u8 rows`, `u8 cols` |
| `0x41` | `GET_COLOR` | `u8 mode`, `u8 led` | `u8 r`, `u8 g`, `u8 b` |
| `0x42` | `SET_COLOR` | `u8 mode`, `u8 led`, `u8 r`, `u8 g`, `u8 b` | ack |
| `0x43` | `GET_KEYCODE` | `u8 mode`, `u8 row`, `u8 col` | `u16 keycode` |
| `0x44` | `SET_KEYCODE` | `u8 mode`, `u8 row`, `u8 col`, `u16 keycode` | ack |
| `0x45` | `COMMIT` | — | ack |
| `0x46` | `RELOAD` | — | ack |
| `0x47` | `RESET_DEFAULTS` | — | ack |

## Firmware behavior

The firmware keeps the existing `keymaps` and `mode_colors` tables as the default source of truth.

On boot it:

1. Reads the QMK user datablock.
2. Validates the QMK datablock header and the dynamic-config `magic`, `version`, `num_modes`, `led_count`, `matrix_rows`, and `matrix_cols`.
3. If the blob is valid, it copies the colors/keymap into RAM.
4. If not, it reseeds RAM from the PROGMEM defaults and writes the blob back to EEPROM.

Runtime updates are two-phase:

- `SET_COLOR` and `SET_KEYCODE` update RAM only.
- `COMMIT` writes the whole RAM blob to EEPROM in one shot.
- `RELOAD` throws away uncommitted RAM edits and re-reads EEPROM.
- `RESET_DEFAULTS` clears the magic so the next boot reseeds from PROGMEM.

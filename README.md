# macropad

This repository provides a Debian trixie base image plus a Compose workflow for building [QMK](https://qmk.fm/) firmware for the [Framework 16](https://frame.work/products/laptop16-diy-amd-7040) RGB macropad, with an editable keymap source tree kept outside the QMK checkout so it stays in your own git history.

The initial keymap demonstrates a **"cycle through N modes" key**: pressing the top-left key steps through five layers, each with a distinct per-key RGB colour pattern defined in a single readable table in `keymap/keymap.c`.

---

## Build

```bash
HOST_UID="$(id -u)" HOST_GID="$(id -g)" docker compose run --rm build-macropad
```

On first run the container clones `FrameworkComputer/qmk_firmware` (branch `fl16-2025`) into `./workspace/qmk_firmware` and initialises submodules — this takes a few minutes.  Subsequent runs reuse the cached checkout.

The compiled firmware lands in `./out/`:

```
./out/framework_macropad_phlax.uf2
```

---

## Flashing

The Framework 16 macropad uses an **RP2040** MCU with a UF2 drag-and-drop bootloader.  Automatic flashing from inside the container is not supported — follow these steps on the **host**:

1. Double-tap the reset button on the back of the macropad.  
   It appears as a USB mass-storage device named **RPI-RP2**.
2. Copy the firmware file onto the drive:
   ```bash
   cp ./out/framework_macropad_phlax.uf2 /run/media/$USER/RPI-RP2/
   ```
   (or drag-and-drop in your file manager)
3. The macropad reboots automatically — done.

> **Tip:** setting `FLASH=1` prints a reminder of these steps rather than attempting anything automatic.

---

## Host directories

| Path | Container mount | Purpose |
|---|---|---|
| `./workspace` | `/workspace` | QMK firmware checkout (cloned on first run) |
| `./out` | `/out` | Compiled firmware (`.uf2`, `.bin`, `.hex`) |
| `./cache` | `/cache` | Build cache (pip, ccache, …) |
| `./keymap` | `/repo/keymap` (read-only) | Your editable keymap source |
| `./scripts` | `/repo/scripts` (read-only) | Build entrypoint script |

All three writable directories (`workspace`, `out`, `cache`) are automatically `chown`-ed to `HOST_UID:HOST_GID` at the end of every run, so files are never left root-owned on the host.

> These directories are listed in `.gitignore` — only `keymap/` is tracked.

---

## Editing the keymap

Everything you iterate on lives in `keymap/`:

| File | Purpose |
|---|---|
| `keymap/keymap.c` | Layer definitions, `CYCLE_MODE` handler, `mode_colors` table |
| `keymap/palette.h` | Named `{R,G,B}` macros (`RGB_RED`, `RGB_GRN`, …) |
| `keymap/config.h` | Per-keymap hardware overrides (brightness, tapping term, …) |
| `keymap/rules.mk` | Feature flags (`RGB_MATRIX_ENABLE`, `MIDI_ENABLE`, …) |

### Changing a key's colour

Open `keymap/keymap.c` and find the `mode_colors` table.  Each entry is one comment + one colour macro:

```c
/* LED  3: Bksp         */ RGB_RED,
```

Swap `RGB_RED` for any macro from `palette.h` (or a literal `{R, G, B}` triple) and rebuild.

### Adding a mode

1. Increment `NUM_MODES` (currently `5`).
2. Add a new `enum layers` entry and a matching `LAYOUT(…)` block in the keymaps array.
3. Add a new row to `mode_colors`.

### LED index ordering

The `mode_colors` table uses LED indices `0–23` in the order defined by the IS31FL3741 driver in `keyboards/framework/macropad/macropad.c` (`g_led_config.matrix_co`).  The approximate mapping is documented in a comment at the top of the table.  If a colour appears on the wrong physical key, compare with that file and swap the table entries to match.

---

## Overriding the QMK source

All parameters are settable as environment variables (with the defaults shown):

| Variable | Default | Description |
|---|---|---|
| `QMK_REPO` | `https://github.com/FrameworkComputer/qmk_firmware.git` | QMK fork to clone |
| `QMK_REF` | `fl16-2025` | Branch, tag, or commit SHA to check out |
| `KEYBOARD` | `framework/macropad` | Keyboard path within `keyboards/` |
| `KEYMAP` | `phlax` | Keymap name |
| `FLASH` | _(empty)_ | Set to any non-empty value to print flash instructions |

Example — pin to a specific commit and use a different keymap name:

```bash
QMK_REF=11c49db KEYMAP=mine \
  HOST_UID="$(id -u)" HOST_GID="$(id -g)" \
  docker compose run --rm build-macropad
```

> **How `KEYBOARD` was determined:** the Framework QMK fork keeps the macropad definition on `fl16-*` branches (not `master`, which is upstream QMK).  Branch `fl16-2025` (commit `11c49db`) contains `keyboards/framework/macropad/` — a 24-key IS31FL3741 RGB pad on an RP2040.  If Framework releases a newer branch, update `QMK_REF` to point to it.

---

## Troubleshooting

**Rebuild the image from scratch** (e.g. after changing the Dockerfile):

```bash
docker compose build --no-cache build-macropad
```

**Re-clone the QMK source** (e.g. to switch branches):

```bash
rm -rf ./workspace/qmk_firmware
HOST_UID="$(id -u)" HOST_GID="$(id -g)" docker compose run --rm build-macropad
```

**`KEYBOARD` path not found / compile error about missing keyboard:**  
Make sure `QMK_REF` points to a Framework branch, not upstream `master`:

```bash
QMK_REF=fl16-2025 HOST_UID="$(id -u)" HOST_GID="$(id -g)" \
  docker compose run --rm build-macropad
```

**Flashing is not automated:**  
The RP2040 UF2 bootloader mounts as a USB drive — `qmk flash` inside a container cannot access it.  Use the manual drag-and-drop steps in the [Flashing](#flashing) section above.
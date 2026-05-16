#!/bin/bash
# build-macropad.sh — compile QMK firmware for the Framework 16 RGB Macropad.

set -euo pipefail

# ── Configuration ─────────────────────────────────────────────────────────────
OUTPUT_DIR="${OUTPUT_DIR:-/out}"
BUILD_CACHE_DIR="${BUILD_CACHE_DIR:-/cache}"
QMK_REPO="${QMK_REPO:-https://github.com/FrameworkComputer/qmk_firmware.git}"
QMK_REF="${QMK_REF:-fl16-2025}"
KEYBOARD="${KEYBOARD:-framework/macropad}"
KEYMAP="${KEYMAP:-phlax}"
FLASH="${FLASH:-}"

QMK_DIR=/workspace/qmk_firmware
QMK=/opt/qmk-venv/bin/qmk

found_files=()

# ── EXIT handler: print outcome ───────────────────────────────────────────────
_cleanup() {
    local rc=$?
    echo ""
    echo "================================================================"
    if [ "$rc" -eq 0 ]; then
        echo "BUILD COMPLETE"
        echo "================================================================"
        printf "  Keyboard : %s\n" "$KEYBOARD"
        printf "  Keymap   : %s\n" "$KEYMAP"
        printf "  QMK ref  : %s\n" "$QMK_REF"
        for f in "${found_files[@]:-}"; do
            printf "  Firmware : %s\n" "$f"
        done
    else
        echo "BUILD FAILED"
        echo "================================================================"
        printf "  Keyboard : %s\n" "$KEYBOARD"
        printf "  Keymap   : %s\n" "$KEYMAP"
        echo "  Review the output above for error details."
    fi
    echo "================================================================"
}
trap _cleanup EXIT

# ── Validate ──────────────────────────────────────────────────────────────────
[ -n "$KEYBOARD" ] || { echo "ERROR: KEYBOARD not set"; exit 1; }
mkdir -p "$OUTPUT_DIR" "$BUILD_CACHE_DIR" /workspace

# ── Clone or update QMK source ────────────────────────────────────────────────
if [ ! -d "$QMK_DIR/.git" ]; then
    echo ">>> Cloning QMK firmware (ref: $QMK_REF) …"
    git clone --branch "$QMK_REF" --depth 1 "$QMK_REPO" "$QMK_DIR"
else
    echo ">>> Updating QMK firmware to $QMK_REF …"
    git -C "$QMK_DIR" fetch origin "$QMK_REF"
    git -C "$QMK_DIR" checkout "$QMK_REF"
fi

# ── Initialise submodules ─────────────────────────────────────────────────────
echo ">>> Initialising git submodules …"
(cd "$QMK_DIR" && make git-submodule)

# ── Symlink keymap into the QMK tree ──────────────────────────────────────────
KEYMAP_TARGET="$QMK_DIR/keyboards/$KEYBOARD/keymaps/$KEYMAP"
echo ">>> Linking /repo/keymap → $KEYMAP_TARGET"
mkdir -p "$(dirname "$KEYMAP_TARGET")"
rm -rf "$KEYMAP_TARGET"
ln -sf /repo/keymap "$KEYMAP_TARGET"

# ── Patch Framework's keyboard source ────────────────────────────────────────
# Framework's macropad.c defines keyboard_post_init_user(), which is the
# callback name reserved for *keymaps*.  It blocks us from defining our own
# (multiple-definition link error).  Stub it out by renaming the symbol so
# our keymap's keyboard_post_init_user() takes over.  The parent
# framework.c's keyboard_post_init_kb() will then call ours.
#
# The only behaviour lost is Framework's "set _NUMLOCK layer on at boot if
# the host has num-lock on" — which referenced a layer name that doesn't
# exist in our keymap anyway.
MACROPAD_C="$QMK_DIR/keyboards/$KEYBOARD/macropad.c"
# Reset any prior patching by checking out a clean copy first.
git -C "$QMK_DIR" checkout -- "keyboards/$KEYBOARD/macropad.c" 2>/dev/null || true
if grep -q '^void keyboard_post_init_user' "$MACROPAD_C"; then
    echo ">>> Patching macropad.c: stubbing keyboard_post_init_user"
    sed -i 's/^void keyboard_post_init_user/__attribute__((unused)) static void _macropad_post_init_unused/' \
        "$MACROPAD_C"
fi

FACTORY_C="$QMK_DIR/keyboards/framework/factory.c"
git -C "$QMK_DIR" checkout -- "keyboards/framework/factory.c" 2>/dev/null || true
python3 - "$FACTORY_C" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()

declaration = 'bool macropad_dyn_config_handle_hid(uint8_t *data, uint8_t length);\n'
if declaration not in text:
    text = text.replace(
        '#endif\n\nenum factory_commands {\n',
        f'#endif\n\n{declaration}\nenum factory_commands {{\n',
        1,
    )

hook = (
    '    if (command_id >= 0x40 && command_id <= 0x4F && macropad_dyn_config_handle_hid(data, length)) {\n'
    '        return true;\n'
    '    }\n\n'
)
needle = '    //uprintf("raw_hid_receive(command: %X, length: %d)\\n", command_id, length);\n\n'
if hook not in text:
    text = text.replace(needle, needle + hook, 1)

path.write_text(text)
PY

# ── Compile ───────────────────────────────────────────────────────────────────
echo ">>> Compiling $KEYBOARD : $KEYMAP …"
cd "$QMK_DIR"
QMK_HOME="$QMK_DIR" "$QMK" compile -kb "$KEYBOARD" -km "$KEYMAP"

# ── Collect firmware artefacts ────────────────────────────────────────────────
FIRMWARE_STEM="${KEYBOARD//\//_}_${KEYMAP}"
for ext in uf2 bin hex; do
    f="$QMK_DIR/${FIRMWARE_STEM}.${ext}"
    if [ -f "$f" ]; then
        cp "$f" "$OUTPUT_DIR/"
        found_files+=("$OUTPUT_DIR/$(basename "$f")")
    fi
done

if [ "${#found_files[@]}" -eq 0 ]; then
    echo "ERROR: No firmware file matching ${FIRMWARE_STEM}.{uf2,bin,hex} found."
    echo "  Check $QMK_DIR/.build/ for partial outputs."
    exit 1
fi

# ── FLASH notice ──────────────────────────────────────────────────────────────
if [ -n "$FLASH" ]; then
    echo ""
    echo "================================================================"
    echo "FLASH NOTE"
    echo "================================================================"
    echo "  The Framework 16 macropad uses an RP2040 with a UF2 bootloader."
    echo "  Copy ./out/${FIRMWARE_STEM}.uf2 onto the RPI-RP2 drive after"
    echo "  double-tapping the reset button on the back of the pad."
    echo "================================================================"
fi

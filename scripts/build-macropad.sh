#!/bin/bash
# build-macropad.sh — compile QMK firmware for the Framework 16 RGB Macropad.
#
# Runs as root inside the build container so it can chown artefacts.
# The `qmk compile` step is executed as the non-root `builder` user.
# Firmware is written to $OUTPUT_DIR; ownership is restored to HOST_UID:HOST_GID.

set -euo pipefail

# ── Configuration ─────────────────────────────────────────────────────────────
OUTPUT_DIR="${OUTPUT_DIR:-/out}"
BUILD_CACHE_DIR="${BUILD_CACHE_DIR:-/cache}"
HOST_UID="${HOST_UID:-1000}"
HOST_GID="${HOST_GID:-1000}"
QMK_REPO="${QMK_REPO:-https://github.com/FrameworkComputer/qmk_firmware.git}"
QMK_REF="${QMK_REF:-fl16-2025}"
KEYBOARD="${KEYBOARD:-framework/macropad}"
KEYMAP="${KEYMAP:-phlax}"
FLASH="${FLASH:-}"

QMK_DIR=/workspace/qmk_firmware
QMK=/home/builder/.venv/bin/qmk

found_files=()

# ── EXIT handler: restore ownership and print outcome ─────────────────────────
_cleanup() {
    local rc=$?
    set +e
    chown -R "${HOST_UID}:${HOST_GID}" "$OUTPUT_DIR" "$BUILD_CACHE_DIR" /workspace \
        2>/dev/null || true
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
        printf "  Keyboard : %s\n" "${KEYBOARD}"
        printf "  Keymap   : %s\n" "${KEYMAP}"
        echo "  Review the output above for error details."
        echo "  Tip: verify KEYBOARD and QMK_REF match the firmware tree, e.g.:"
        echo "    KEYBOARD=framework/macropad QMK_REF=fl16-2025 \\"
        echo "      docker compose run --rm build-macropad"
    fi
    echo "================================================================"
}
trap _cleanup EXIT

# ── Validate required variables ───────────────────────────────────────────────
if [ -z "$KEYBOARD" ]; then
    echo "ERROR: KEYBOARD is not set."
    echo "  Set it as an environment variable before running, e.g.:"
    echo "    KEYBOARD=framework/macropad docker compose run --rm build-macropad"
    exit 1
fi

# ── Ensure output directories exist ──────────────────────────────────────────
mkdir -p "$OUTPUT_DIR" "$BUILD_CACHE_DIR"

# ── Clone or update QMK source ───────────────────────────────────────────────
if [ ! -d "$QMK_DIR/.git" ]; then
    echo ">>> Cloning QMK firmware (ref: $QMK_REF) …"
    git clone --branch "$QMK_REF" --depth 1 "$QMK_REPO" "$QMK_DIR"
else
    echo ">>> Updating QMK firmware to $QMK_REF …"
    git -C "$QMK_DIR" fetch origin "$QMK_REF"
    git -C "$QMK_DIR" checkout "$QMK_REF"
fi

# ── Initialise submodules ────────────────────────────────────────────────────
echo ">>> Initialising git submodules …"
(cd "$QMK_DIR" && make git-submodule)

# ── Symlink keymap into the QMK tree ─────────────────────────────────────────
KEYMAP_TARGET="$QMK_DIR/keyboards/$KEYBOARD/keymaps/$KEYMAP"
echo ">>> Linking /repo/keymap → $KEYMAP_TARGET"
mkdir -p "$(dirname "$KEYMAP_TARGET")"
rm -rf "$KEYMAP_TARGET"
ln -sf /repo/keymap "$KEYMAP_TARGET"

# ── Fix workspace ownership so builder can compile ───────────────────────────
chown -R builder:builder "$QMK_DIR"

# ── Compile ──────────────────────────────────────────────────────────────────
echo ">>> Compiling $KEYBOARD : $KEYMAP …"
runuser -u builder -- bash -c "
    export HOME=/home/builder
    export QMK_HOME=$QMK_DIR
    cd $QMK_DIR
    $QMK compile -kb '$KEYBOARD' -km '$KEYMAP'
"

# ── Collect firmware artefacts ───────────────────────────────────────────────
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

# ── FLASH notice ─────────────────────────────────────────────────────────────
if [ -n "$FLASH" ]; then
    echo ""
    echo "================================================================"
    echo "FLASH NOTE"
    echo "================================================================"
    echo "  The Framework 16 macropad uses an RP2040 with a UF2 bootloader."
    echo "  Automatic flashing from inside the container is NOT supported."
    echo ""
    echo "  To flash:"
    echo "    1. Double-tap the reset button on the back of the macropad."
    echo "       It will appear as a USB mass-storage device (RPI-RP2)."
    echo "    2. Copy the firmware onto the drive:"
    echo "         cp ./out/${FIRMWARE_STEM}.uf2 /run/media/<user>/RPI-RP2/"
    echo "       (or drag-and-drop in your file manager)"
    echo "    3. The macropad reboots automatically — done."
    echo "================================================================"
fi

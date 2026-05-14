// keymap.c — Framework 16 RGB Macropad — phlax keymap
//
// Keyboard : framework/macropad  (keyboards/framework/macropad/)
// MCU      : RP2040
// LEDs     : 24 × IS31FL3741 per-key RGB
//
// Key features:
//   • 5 layers (_MODE0 .. _MODE4) cycled by pressing CYCLE_MODE (top-left key).
//   • Per-key, per-mode colours defined in mode_colors[][RGB_MATRIX_LED_COUNT][3].
//     Edit a single line in that table to change any key's colour on any mode.
//   • LED index ordering follows the IS31FL3741 CS/SW wiring in macropad.c.
//     Cross-reference g_led_config.matrix_co[row][col] there if colours appear
//     on the wrong keys — adjust the table entries accordingly.

#include QMK_KEYBOARD_H
#include "palette.h"

// ── Layers ───────────────────────────────────────────────────────────────────

#define NUM_MODES 5

enum layers {
    _MODE0 = 0,
    _MODE1,
    _MODE2,
    _MODE3,
    _MODE4,
};

// ── Custom keycodes ──────────────────────────────────────────────────────────

enum custom_keycodes {
    CYCLE_MODE = SAFE_RANGE,
};

// ── Keymaps ──────────────────────────────────────────────────────────────────
//
// Physical layout (6 rows × 4 columns = 24 keys):
//
//   ┌──────────┬──────────┬──────────┬──────────┐
//   │  H1      │  H2      │  H3      │  H4      │  ← header row
//   │CYCLE_MODE│  Calc    │   =      │  Bksp    │
//   ├──────────┼──────────┼──────────┼──────────┤
//   │  K90     │  K95     │  K100    │  K105    │
//   │  NumLk   │   /      │   *      │   -      │
//   ├──────────┼──────────┼──────────┼──────────┤
//   │  K91     │  K96     │  K101    │  K107    │
//   │   7      │   8      │   9      │   +      │
//   ├──────────┼──────────┼──────────┤  (tall)  │
//   │  K92     │  K97     │  K102    │  K106    │
//   │   4      │   5      │   6      │          │
//   ├──────────┼──────────┼──────────┼──────────┤
//   │  K93     │  K98     │  K103    │  K109    │
//   │   1      │   2      │   3      │  Enter   │
//   ├──────────┴──────────┼──────────┤  (tall)  │
//   │  K99 / K110         │  K104    │  K108    │
//   │   0  (wide)         │   .      │          │
//   └─────────────────────┴──────────┴──────────┘
//
// TODO: If the LAYOUT macro name has changed in the upstream keyboard source,
//       replace LAYOUT below with the correct macro from macropad.h.
//       All 5 layers must provide exactly 24 entries in the same positional order.

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    // ── Mode 0: standard numpad ──────────────────────────────────────────────
    // CYCLE_MODE sits in the Esc position (H1, top-left).
    [_MODE0] = LAYOUT(
        CYCLE_MODE, KC_CALC, KC_PEQL, KC_BSPC,
        KC_NUM,     KC_PSLS, KC_PAST, KC_PMNS,
        KC_P7,      KC_P8,   KC_P9,   KC_PPLS,
        KC_P4,      KC_P5,   KC_P6,   KC_NO,
        KC_P1,      KC_P2,   KC_P3,   KC_PENT,
        KC_P0,      KC_NO,   KC_PDOT, KC_NO
    ),

    // ── Modes 1–4: placeholder — edit to taste ───────────────────────────────
    // H1 (top-left) cycles mode on every layer.
    // Fill in the other 23 positions with whatever keycodes you need.
    [_MODE1] = LAYOUT(
        CYCLE_MODE, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_NO,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_NO,   KC_TRNS, KC_NO
    ),

    [_MODE2] = LAYOUT(
        CYCLE_MODE, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_NO,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_NO,   KC_TRNS, KC_NO
    ),

    [_MODE3] = LAYOUT(
        CYCLE_MODE, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_NO,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_NO,   KC_TRNS, KC_NO
    ),

    [_MODE4] = LAYOUT(
        CYCLE_MODE, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_NO,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_NO,   KC_TRNS, KC_NO
    ),
};

// ── Per-key, per-mode colour table ───────────────────────────────────────────
//
// Indexed [mode][led_index][R/G/B].
// LED indices 0–23 follow the IS31FL3741 CS/SW order defined in macropad.c's
// g_is31_leds[] / g_led_config arrays.  Approximate mapping to physical keys:
//
//   LED  0  1  2  3   → header row : CYCLE_MODE, Calc, =, Bksp
//   LED  4  5  6  7   → row 1      : NumLk, /, *, -
//   LED  8  9 10 11   → row 2      : 7, 8, 9, + (upper)
//   LED 12 13 14 15   → row 3      : 4, 5, 6, + (lower / tall-key continuation)
//   LED 16 17 18 19   → row 4      : 1, 2, 3, Enter (upper)
//   LED 20 21 22 23   → row 5      : 0 (left), 0 (right/wide), ., Enter (lower)
//
// If colours appear on the wrong physical keys, compare with
//   keyboards/framework/macropad/macropad.c  → g_led_config.matrix_co[][]
// and swap the entries in this table to match.
//
// Editing guide: each row is one LED.  Change a single {R,G,B} triple (or swap
// a palette macro) to recolour that key on that mode.

static const uint8_t mode_colors[NUM_MODES][RGB_MATRIX_LED_COUNT][3] = {

    /* ── Mode 0: numpad — digits white, operators blue, Bksp red, Enter/+ green */
    [_MODE0] = {
        /* LED  0: CYCLE_MODE   */ RGB_MAG,
        /* LED  1: Calc         */ RGB_YEL,
        /* LED  2: =            */ RGB_CYN,
        /* LED  3: Bksp         */ RGB_RED,
        /* LED  4: NumLk        */ RGB_ORG,
        /* LED  5: /            */ RGB_BLU,
        /* LED  6: *            */ RGB_BLU,
        /* LED  7: -            */ RGB_BLU,
        /* LED  8: 7            */ RGB_WHT,
        /* LED  9: 8            */ RGB_WHT,
        /* LED 10: 9            */ RGB_WHT,
        /* LED 11: + (upper)    */ RGB_GRN,
        /* LED 12: 4            */ RGB_WHT,
        /* LED 13: 5            */ RGB_WHT,
        /* LED 14: 6            */ RGB_WHT,
        /* LED 15: + (lower)    */ RGB_GRN,
        /* LED 16: 1            */ RGB_WHT,
        /* LED 17: 2            */ RGB_WHT,
        /* LED 18: 3            */ RGB_WHT,
        /* LED 19: Enter (upper)*/ RGB_GRN,
        /* LED 20: 0 (left)     */ RGB_WHT,
        /* LED 21: 0 (right)    */ RGB_WHT,
        /* LED 22: .            */ RGB_WHT,
        /* LED 23: Enter (lower)*/ RGB_GRN,
    },

    /* ── Mode 1: all red */
    [_MODE1] = {
        RGB_RED, RGB_RED, RGB_RED, RGB_RED,
        RGB_RED, RGB_RED, RGB_RED, RGB_RED,
        RGB_RED, RGB_RED, RGB_RED, RGB_RED,
        RGB_RED, RGB_RED, RGB_RED, RGB_RED,
        RGB_RED, RGB_RED, RGB_RED, RGB_RED,
        RGB_RED, RGB_RED, RGB_RED, RGB_RED,
    },

    /* ── Mode 2: all green */
    [_MODE2] = {
        RGB_GRN, RGB_GRN, RGB_GRN, RGB_GRN,
        RGB_GRN, RGB_GRN, RGB_GRN, RGB_GRN,
        RGB_GRN, RGB_GRN, RGB_GRN, RGB_GRN,
        RGB_GRN, RGB_GRN, RGB_GRN, RGB_GRN,
        RGB_GRN, RGB_GRN, RGB_GRN, RGB_GRN,
        RGB_GRN, RGB_GRN, RGB_GRN, RGB_GRN,
    },

    /* ── Mode 3: rainbow rows (red → orange → yellow → green → blue → magenta) */
    [_MODE3] = {
        RGB_RED, RGB_RED, RGB_RED, RGB_RED,   /* row 0 */
        RGB_ORG, RGB_ORG, RGB_ORG, RGB_ORG,   /* row 1 */
        RGB_YEL, RGB_YEL, RGB_YEL, RGB_YEL,   /* row 2 */
        RGB_GRN, RGB_GRN, RGB_GRN, RGB_GRN,   /* row 3 */
        RGB_BLU, RGB_BLU, RGB_BLU, RGB_BLU,   /* row 4 */
        RGB_MAG, RGB_MAG, RGB_MAG, RGB_MAG,   /* row 5 */
    },

    /* ── Mode 4: all off */
    [_MODE4] = {
        RGB_OFF, RGB_OFF, RGB_OFF, RGB_OFF,
        RGB_OFF, RGB_OFF, RGB_OFF, RGB_OFF,
        RGB_OFF, RGB_OFF, RGB_OFF, RGB_OFF,
        RGB_OFF, RGB_OFF, RGB_OFF, RGB_OFF,
        RGB_OFF, RGB_OFF, RGB_OFF, RGB_OFF,
        RGB_OFF, RGB_OFF, RGB_OFF, RGB_OFF,
    },
};

// ── Custom keycode handler ────────────────────────────────────────────────────

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == CYCLE_MODE && record->event.pressed) {
        uint8_t next = (get_highest_layer(layer_state) + 1) % NUM_MODES;
        layer_move(next);
    }
    return true;
}

// ── RGB Matrix initialisation ────────────────────────────────────────────────
//
// Set a static base effect so the indicator callback owns every frame.
// Without this, the default animation fights the per-key colours.

void keyboard_post_init_user(void) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
}

// ── Per-key, per-mode RGB indicator ─────────────────────────────────────────

bool rgb_matrix_indicators_user(void) {
    uint8_t m = get_highest_layer(layer_state);
    if (m >= NUM_MODES) return false;

    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        rgb_matrix_set_color(i,
            mode_colors[m][i][0],
            mode_colors[m][i][1],
            mode_colors[m][i][2]);
    }
    return false;
}

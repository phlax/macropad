// keymap.c — Framework 16 RGB Macropad — phlax keymap
//
// Physical layout (top-to-bottom as you look at the pad):
//   Top row:    MODE   --      --      --     (rightmost = QK_BOOT in mode 4)
//               *      +       -       Bksp
//               7      8       9       |
//               4      5       6       PgUp
//               1      2       3       PgDn
//   Bottom row: 0      Home    End     Enter

#include QMK_KEYBOARD_H
#include "palette.h"

#define NUM_MODES 5

enum layers {
    _MODE0 = 0,
    _MODE1,
    _MODE2,
    _MODE3,
    _MODE4,
};

enum custom_keycodes {
    CYCLE_MODE = SAFE_RANGE,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_MODE0] = LAYOUT(
        CYCLE_MODE, KC_F13,  KC_F14,  KC_F15,
        KC_PAST,    KC_PMNS, KC_PPLS, KC_BSPC,
        KC_7,       KC_8,    KC_9,    S(KC_NUBS),
        KC_4,       KC_5,    KC_6,    KC_HOME,
        KC_1,       KC_2,    KC_3,    KC_END,
        KC_PGUP,    KC_0,    KC_PGDN, KC_ENT
    ),

    [_MODE1] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    [_MODE2] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    [_MODE3] = LAYOUT(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    ),

    // Mode 4: top row's rightmost slot becomes QK_BOOT.
    [_MODE4] = LAYOUT(
        CYCLE_MODE, KC_NO,   KC_NO,   QK_BOOT,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS,    KC_TRNS, KC_TRNS, KC_TRNS
    ),
};

// LED-index → physical position (from Framework's wiring).  Row 0 = top row.
//
//   LED  0 → r1c2 = "+"
//   LED  1 → r2c2 = "8"
//   LED  2 → r0c2 = ---       (top row, 2nd)
//   LED  3 → r3c2 = "5"
//   LED  4 → r1c1 = "*"
//   LED  5 → r0c1 = "MODE"    (top-left)
//   LED  6 → r3c1 = "4"
//   LED  7 → r2c1 = "7"
//   LED  8 → r5c1 = "0"       (bottom-left)
//   LED  9 → r4c1 = "1"
//   LED 10 → r5c2 = "Home"    (bottom row, 2nd)
//   LED 11 → r4c2 = "2"
//   LED 12 → r5c4 = "Enter"   (bottom-right)
//   LED 13 → r4c4 = "PgDn"
//   LED 14 → r5c3 = "End"
//   LED 15 → r4c3 = "3"
//   LED 16 → r2c4 = "|"
//   LED 17 → r0c4 = ---       (top-right, QK_BOOT in mode 4)
//   LED 18 → r1c4 = "Bksp"
//   LED 19 → r3c4 = "PgUp"
//   LED 20 → r1c3 = "-"
//   LED 21 → r2c3 = "9"
//   LED 22 → r0c3 = ---       (top row, 3rd)
//   LED 23 → r3c3 = "6"

static const uint8_t mode_colors[NUM_MODES][RGB_MATRIX_LED_COUNT][3] = {

    [_MODE0] = {
        /* LED  0: +     */ PAL_YEL,
        /* LED  1: 8     */ PAL_WHT,
        /* LED  2: ---   */ PAL_OFF,
        /* LED  3: 5     */ PAL_WHT,
        /* LED  4: *     */ PAL_YEL,
        /* LED  5: MODE  */ PAL_MAG,
        /* LED  6: 4     */ PAL_WHT,
        /* LED  7: 7     */ PAL_WHT,
        /* LED  8: PgUp  */ PAL_CYN,
        /* LED  9: 1     */ PAL_WHT,
        /* LED 10: 0     */ PAL_WHT,
        /* LED 11: 2     */ PAL_WHT,
        /* LED 12: Enter */ PAL_GRN,
        /* LED 13: End   */ PAL_CYN,
        /* LED 14: PgDn  */ PAL_CYN,
        /* LED 15: 3     */ PAL_WHT,
        /* LED 16: |     */ PAL_YEL,
        /* LED 17: ---   */ PAL_OFF,
        /* LED 18: Bksp  */ PAL_RED,
        /* LED 19: Home  */ PAL_CYN,
        /* LED 20: -     */ PAL_YEL,
        /* LED 21: 9     */ PAL_WHT,
        /* LED 22: ---   */ PAL_OFF,
        /* LED 23: 6     */ PAL_WHT,
    },

    [_MODE1] = {
        PAL_RED, PAL_RED, PAL_RED, PAL_RED,
        PAL_RED, PAL_RED, PAL_RED, PAL_RED,
        PAL_RED, PAL_RED, PAL_RED, PAL_RED,
        PAL_RED, PAL_RED, PAL_RED, PAL_RED,
        PAL_RED, PAL_RED, PAL_RED, PAL_RED,
        PAL_RED, PAL_RED, PAL_RED, PAL_RED,
    },

    [_MODE2] = {
        PAL_GRN, PAL_GRN, PAL_GRN, PAL_GRN,
        PAL_GRN, PAL_GRN, PAL_GRN, PAL_GRN,
        PAL_GRN, PAL_GRN, PAL_GRN, PAL_GRN,
        PAL_GRN, PAL_GRN, PAL_GRN, PAL_GRN,
        PAL_GRN, PAL_GRN, PAL_GRN, PAL_GRN,
        PAL_GRN, PAL_GRN, PAL_GRN, PAL_GRN,
    },

    // Mode 3: horizontal rainbow stripes, top-to-bottom red→magenta.
    [_MODE3] = {
        /* LED  0: row 1 */ PAL_ORG,
        /* LED  1: row 2 */ PAL_YEL,
        /* LED  2: row 0 */ PAL_RED,
        /* LED  3: row 3 */ PAL_GRN,
        /* LED  4: row 1 */ PAL_ORG,
        /* LED  5: row 0 */ PAL_RED,
        /* LED  6: row 3 */ PAL_GRN,
        /* LED  7: row 2 */ PAL_YEL,
        /* LED  8: row 5 */ PAL_MAG,
        /* LED  9: row 4 */ PAL_BLU,
        /* LED 10: row 5 */ PAL_MAG,
        /* LED 11: row 4 */ PAL_BLU,
        /* LED 12: row 5 */ PAL_MAG,
        /* LED 13: row 4 */ PAL_BLU,
        /* LED 14: row 5 */ PAL_MAG,
        /* LED 15: row 4 */ PAL_BLU,
        /* LED 16: row 2 */ PAL_YEL,
        /* LED 17: row 0 */ PAL_RED,
        /* LED 18: row 1 */ PAL_ORG,
        /* LED 19: row 3 */ PAL_GRN,
        /* LED 20: row 1 */ PAL_ORG,
        /* LED 21: row 2 */ PAL_YEL,
        /* LED 22: row 0 */ PAL_RED,
        /* LED 23: row 3 */ PAL_GRN,
    },

    // Mode 4: all off, except LED 17 (top-right, QK_BOOT) glowing red.
    [_MODE4] = {
        PAL_OFF, PAL_OFF, PAL_OFF, PAL_OFF,
        PAL_OFF, PAL_OFF, PAL_OFF, PAL_OFF,
        PAL_OFF, PAL_OFF, PAL_OFF, PAL_OFF,
        PAL_OFF, PAL_OFF, PAL_OFF, PAL_OFF,
        PAL_OFF, PAL_RED, PAL_OFF, PAL_OFF,
        PAL_OFF, PAL_OFF, PAL_OFF, PAL_OFF,
    },
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == CYCLE_MODE && record->event.pressed) {
        uint8_t next = (get_highest_layer(layer_state) + 1) % NUM_MODES;
        layer_move(next);
    }
    return true;
}

void keyboard_post_init_user(void) {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
}

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

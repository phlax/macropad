#pragma once
// config.h — per-keymap overrides for the Framework 16 RGB Macropad keymap.
//
// This file is compiled before the keyboard's own config.h so values set here
// take precedence.  Add per-keymap hardware tweaks here rather than editing the
// upstream QMK keyboard source.
//
// Examples:
//   #define RGB_MATRIX_DEFAULT_VAL      128   // default brightness (0–255)
//   #define RGB_MATRIX_DEFAULT_MODE     RGB_MATRIX_SOLID_COLOR
//   #define TAPPING_TERM                200   // tap-hold threshold in ms
//   #define PERMISSIVE_HOLD                   // resolve holds faster

#ifndef NUM_MODES
#    define NUM_MODES 5
#endif

#define EECONFIG_USER_DATA_SIZE 4096

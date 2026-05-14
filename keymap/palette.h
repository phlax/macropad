#pragma once
/*
 * palette.h — named RGB colour macros for use in the mode_colors table.
 *
 * Each macro expands to a braced {R, G, B} triple usable directly as an
 * element of the uint8_t[][3] mode_colors array in keymap.c.
 *
 * Named PAL_* rather than RGB_* to avoid collision with QMK's own
 * comma-separated RGB_* macros in quantum/color.h.
 */

#define PAL_OFF   {  0,   0,   0}
#define PAL_RED   {255,   0,   0}
#define PAL_GRN   {  0, 255,   0}
#define PAL_BLU   {  0,   0, 255}
#define PAL_YEL   {255, 255,   0}
#define PAL_MAG   {255,   0, 255}
#define PAL_CYN   {  0, 255, 255}
#define PAL_WHT   {255, 255, 255}
#define PAL_ORG   {255, 128,   0}

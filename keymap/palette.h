#pragma once
/*
 * palette.h — named RGB colour macros for use in the mode_colors table.
 *
 * Each macro expands to a braced {R, G, B} triple and can be used directly
 * as an element of the uint8_t[][3] mode_colors array in keymap.c.
 */

#define RGB_OFF   {  0,   0,   0}
#define RGB_RED   {255,   0,   0}
#define RGB_GRN   {  0, 255,   0}
#define RGB_BLU   {  0,   0, 255}
#define RGB_YEL   {255, 255,   0}
#define RGB_MAG   {255,   0, 255}
#define RGB_CYN   {  0, 255, 255}
#define RGB_WHT   {255, 255, 255}
#define RGB_ORG   {255, 128,   0}

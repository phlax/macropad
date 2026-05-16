#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifndef NUM_MODES
#    define NUM_MODES 5
#endif

#define DYN_CONFIG_MAGIC   0x4D504344UL
#define DYN_CONFIG_VERSION 0x0001

#define CMD_GET_INFO          0x40
#define CMD_GET_COLOR         0x41
#define CMD_SET_COLOR         0x42
#define CMD_GET_KEYCODE       0x43
#define CMD_SET_KEYCODE       0x44
#define CMD_COMMIT            0x45
#define CMD_RELOAD            0x46
#define CMD_RESET_TO_DEFAULTS 0x47

typedef struct dyn_config {
    uint32_t magic;
    uint16_t version;
    uint16_t num_modes;
    uint16_t led_count;
    uint8_t  matrix_rows;
    uint8_t  matrix_cols;
    uint8_t  colors[NUM_MODES][RGB_MATRIX_LED_COUNT][3];
    uint16_t keymap[NUM_MODES][MATRIX_ROWS][MATRIX_COLS];
} dyn_config_t;

extern dyn_config_t g_dyn_config;

void dyn_config_init(void);
void dyn_config_commit(void);
void dyn_config_reload(void);
void dyn_config_invalidate_eeprom(void);
bool macropad_dyn_config_handle_hid(uint8_t *data, uint8_t length);

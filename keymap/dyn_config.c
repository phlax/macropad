#include QMK_KEYBOARD_H

#include <string.h>

#include "dyn_config.h"
#include "eeconfig.h"
#include "raw_hid.h"

extern const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS];
extern const uint8_t PROGMEM mode_colors[NUM_MODES][RGB_MATRIX_LED_COUNT][3];

dyn_config_t g_dyn_config;

static uint8_t g_dyn_config_eeprom[EECONFIG_USER_DATA_SIZE];

_Static_assert(NUM_MODES <= UINT8_MAX, "NUM_MODES must fit in protocol fields");
_Static_assert(RGB_MATRIX_LED_COUNT <= UINT8_MAX, "RGB_MATRIX_LED_COUNT must fit in protocol fields");
_Static_assert(sizeof(dyn_config_t) <= EECONFIG_USER_DATA_SIZE, "dyn_config_t must fit in EECONFIG_USER_DATA_SIZE");

static uint16_t read_u16_le(const uint8_t *data) {
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static void write_u16_le(uint8_t *data, uint16_t value) {
    data[0] = value & 0xFF;
    data[1] = value >> 8;
}

static void dyn_config_sync_header(void) {
    g_dyn_config.magic       = DYN_CONFIG_MAGIC;
    g_dyn_config.version     = DYN_CONFIG_VERSION;
    g_dyn_config.num_modes   = NUM_MODES;
    g_dyn_config.led_count   = RGB_MATRIX_LED_COUNT;
    g_dyn_config.matrix_rows = MATRIX_ROWS;
    g_dyn_config.matrix_cols = MATRIX_COLS;
}

static bool dyn_config_matches_runtime(const dyn_config_t *config) {
    return config->magic == DYN_CONFIG_MAGIC &&
           config->version == DYN_CONFIG_VERSION &&
           config->num_modes == NUM_MODES &&
           config->led_count == RGB_MATRIX_LED_COUNT &&
           config->matrix_rows == MATRIX_ROWS &&
           config->matrix_cols == MATRIX_COLS;
}

static void dyn_config_seed_defaults(void) {
    memset(&g_dyn_config, 0, sizeof(g_dyn_config));
    dyn_config_sync_header();

    for (uint8_t mode = 0; mode < NUM_MODES; mode++) {
        for (uint8_t led = 0; led < RGB_MATRIX_LED_COUNT; led++) {
            for (uint8_t channel = 0; channel < 3; channel++) {
                g_dyn_config.colors[mode][led][channel] = pgm_read_byte(&mode_colors[mode][led][channel]);
            }
        }

        for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
            for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                g_dyn_config.keymap[mode][row][col] = pgm_read_word(&keymaps[mode][row][col]);
            }
        }
    }
}

static void dyn_config_send_reply(uint8_t command_id, uint8_t request_length, const uint8_t *payload, uint8_t payload_len) {
    uint8_t response[RAW_EPSIZE] = {0};

    response[0] = command_id;
    if (payload != NULL && payload_len > 0) {
        memcpy(&response[1], payload, payload_len);
    }

    raw_hid_send(response, request_length);
}

static void dyn_config_load(bool persist_defaults) {
    memset(g_dyn_config_eeprom, 0, sizeof(g_dyn_config_eeprom));
    eeconfig_read_user_datablock(g_dyn_config_eeprom);
    memcpy(&g_dyn_config, g_dyn_config_eeprom, sizeof(g_dyn_config));

    if (!eeconfig_is_user_datablock_valid() || !dyn_config_matches_runtime(&g_dyn_config)) {
        dyn_config_seed_defaults();
        if (persist_defaults) {
            dyn_config_commit();
        }
    }
}

void dyn_config_init(void) {
    dyn_config_load(true);
}

void dyn_config_commit(void) {
    dyn_config_sync_header();
    memset(g_dyn_config_eeprom, 0, sizeof(g_dyn_config_eeprom));
    memcpy(g_dyn_config_eeprom, &g_dyn_config, sizeof(g_dyn_config));
    eeconfig_update_user_datablock(g_dyn_config_eeprom);
}

void dyn_config_reload(void) {
    dyn_config_load(false);
}

void dyn_config_invalidate_eeprom(void) {
    memset(g_dyn_config_eeprom, 0, sizeof(g_dyn_config_eeprom));
    eeconfig_update_user_datablock(g_dyn_config_eeprom);
}

uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key) {
    if (layer < NUM_MODES && key.row < MATRIX_ROWS && key.col < MATRIX_COLS) {
        return g_dyn_config.keymap[layer][key.row][key.col];
    }

    return KC_NO;
}

bool macropad_dyn_config_handle_hid(uint8_t *data, uint8_t length) {
    uint8_t payload[8] = {0};
    uint8_t mode;

    if (length < RAW_EPSIZE) {
        return false;
    }

    switch (data[0]) {
        case CMD_GET_INFO:
            write_u16_le(&payload[0], DYN_CONFIG_VERSION);
            write_u16_le(&payload[2], NUM_MODES);
            write_u16_le(&payload[4], RGB_MATRIX_LED_COUNT);
            payload[6] = MATRIX_ROWS;
            payload[7] = MATRIX_COLS;
            dyn_config_send_reply(CMD_GET_INFO, length, payload, sizeof(payload));
            return true;

        case CMD_GET_COLOR:
            mode = data[1];
            if (mode < NUM_MODES && data[2] < RGB_MATRIX_LED_COUNT) {
                payload[0] = g_dyn_config.colors[mode][data[2]][0];
                payload[1] = g_dyn_config.colors[mode][data[2]][1];
                payload[2] = g_dyn_config.colors[mode][data[2]][2];
            }
            dyn_config_send_reply(CMD_GET_COLOR, length, payload, 3);
            return true;

        case CMD_SET_COLOR:
            mode = data[1];
            if (mode < NUM_MODES && data[2] < RGB_MATRIX_LED_COUNT) {
                g_dyn_config.colors[mode][data[2]][0] = data[3];
                g_dyn_config.colors[mode][data[2]][1] = data[4];
                g_dyn_config.colors[mode][data[2]][2] = data[5];
            }
            dyn_config_send_reply(CMD_SET_COLOR, length, NULL, 0);
            return true;

        case CMD_GET_KEYCODE:
            mode = data[1];
            if (mode < NUM_MODES && data[2] < MATRIX_ROWS && data[3] < MATRIX_COLS) {
                write_u16_le(payload, g_dyn_config.keymap[mode][data[2]][data[3]]);
            }
            dyn_config_send_reply(CMD_GET_KEYCODE, length, payload, 2);
            return true;

        case CMD_SET_KEYCODE:
            mode = data[1];
            if (mode < NUM_MODES && data[2] < MATRIX_ROWS && data[3] < MATRIX_COLS) {
                g_dyn_config.keymap[mode][data[2]][data[3]] = read_u16_le(&data[4]);
            }
            dyn_config_send_reply(CMD_SET_KEYCODE, length, NULL, 0);
            return true;

        case CMD_COMMIT:
            dyn_config_commit();
            dyn_config_send_reply(CMD_COMMIT, length, NULL, 0);
            return true;

        case CMD_RELOAD:
            dyn_config_reload();
            dyn_config_send_reply(CMD_RELOAD, length, NULL, 0);
            return true;

        case CMD_RESET_TO_DEFAULTS:
            dyn_config_invalidate_eeprom();
            dyn_config_send_reply(CMD_RESET_TO_DEFAULTS, length, NULL, 0);
            return true;

        default:
            return false;
    }
}

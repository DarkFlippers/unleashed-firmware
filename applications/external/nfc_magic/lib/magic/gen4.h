#pragma once

#include <lib/nfc/protocols/mifare_classic.h>

#define MAGIC_GEN4_DEFAULT_PWD 0x00000000
#define MAGIC_GEN4_CONFIG_LEN 32

#define NFCID1_SINGLE_SIZE 4
#define NFCID1_DOUBLE_SIZE 7
#define NFCID1_TRIPLE_SIZE 10

typedef enum {
    MagicGen4UIDLengthSingle = 0x00,
    MagicGen4UIDLengthDouble = 0x01,
    MagicGen4UIDLengthTriple = 0x02
} MagicGen4UIDLength;

typedef enum {
    MagicGen4UltralightModeUL_EV1 = 0x00,
    MagicGen4UltralightModeNTAG = 0x01,
    MagicGen4UltralightModeUL_C = 0x02,
    MagicGen4UltralightModeUL = 0x03
} MagicGen4UltralightMode;

typedef enum {
    // for writing original (shadow) data
    MagicGen4ShadowModePreWrite = 0x00,
    // written data can be read once before restored to original
    MagicGen4ShadowModeRestore = 0x01,
    // written data is discarded
    MagicGen4ShadowModeIgnore = 0x02,
    // apparently for UL?
    MagicGen4ShadowModeHighSpeedIgnore = 0x03
} MagicGen4ShadowMode;

bool magic_gen4_get_cfg(uint32_t pwd, uint8_t* config);

bool magic_gen4_set_cfg(uint32_t pwd, const uint8_t* config, uint8_t config_length, bool fuse);

bool magic_gen4_set_pwd(uint32_t old_pwd, uint32_t new_pwd);

bool magic_gen4_read_blk(uint32_t pwd, uint8_t block_num, uint8_t* data);

bool magic_gen4_write_blk(uint32_t pwd, uint8_t block_num, const uint8_t* data);

bool magic_gen4_wipe(uint32_t pwd);

void magic_gen4_deactivate();

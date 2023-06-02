#pragma once

#include <lib/nfc/protocols/mifare_classic.h>

bool magic_gen1_wupa();

bool magic_gen1_read_block(uint8_t block_num, MfClassicBlock* data);

bool magic_gen1_data_access_cmd();

bool magic_gen1_write_blk(uint8_t block_num, MfClassicBlock* data);

#pragma once

#include <stdbool.h>
#include <storage/storage.h>
#include <lib/toolbox/stream/file_stream.h>

bool nfc_mf_classic_dict_check_presence(Storage* storage);

bool nfc_mf_classic_dict_open_file(Stream* stream);

void nfc_mf_classic_dict_close_file(Stream* stream);

bool nfc_mf_classic_dict_get_next_key(Stream* stream, uint64_t* key);

void nfc_mf_classic_dict_reset(Stream* stream);

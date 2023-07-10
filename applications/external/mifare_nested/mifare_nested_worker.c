#include "mifare_nested_worker_i.h"

#include "lib/nested/nested.h"
#include "lib/parity/parity.h"
#include <lib/nfc/protocols/nfc_util.h>

#include <storage/storage.h>
#include <stream/stream.h>
#include <stream/file_stream.h>
#include "string.h"
#include <furi.h>
#include <furi_hal.h>

#define TAG "MifareNestedWorker"

// possible sum property values
static uint16_t sums[] =
    {0, 32, 56, 64, 80, 96, 104, 112, 120, 128, 136, 144, 152, 160, 176, 192, 200, 224, 256};

void mifare_nested_worker_change_state(
    MifareNestedWorker* mifare_nested_worker,
    MifareNestedWorkerState state) {
    furi_assert(mifare_nested_worker);

    mifare_nested_worker->state = state;
}

MifareNestedWorker* mifare_nested_worker_alloc() {
    MifareNestedWorker* mifare_nested_worker = malloc(sizeof(MifareNestedWorker));

    // Worker thread attributes
    mifare_nested_worker->thread = furi_thread_alloc_ex(
        "MifareNestedWorker", 8192, mifare_nested_worker_task, mifare_nested_worker);

    mifare_nested_worker->callback = NULL;
    mifare_nested_worker->context = NULL;

    mifare_nested_worker_change_state(mifare_nested_worker, MifareNestedWorkerStateReady);

    return mifare_nested_worker;
}

void mifare_nested_worker_free(MifareNestedWorker* mifare_nested_worker) {
    furi_assert(mifare_nested_worker);

    furi_thread_free(mifare_nested_worker->thread);
    free(mifare_nested_worker);
}

void mifare_nested_worker_stop(MifareNestedWorker* mifare_nested_worker) {
    furi_assert(mifare_nested_worker);

    mifare_nested_worker_change_state(mifare_nested_worker, MifareNestedWorkerStateStop);
    furi_thread_join(mifare_nested_worker->thread);
}

void mifare_nested_worker_start(
    MifareNestedWorker* mifare_nested_worker,
    MifareNestedWorkerState state,
    NfcDeviceData* dev_data,
    MifareNestedWorkerCallback callback,
    void* context) {
    furi_assert(mifare_nested_worker);
    furi_assert(dev_data);

    mifare_nested_worker->callback = callback;
    mifare_nested_worker->context = context;
    mifare_nested_worker->dev_data = dev_data;
    mifare_nested_worker_change_state(mifare_nested_worker, state);
    furi_thread_start(mifare_nested_worker->thread);
}

int32_t mifare_nested_worker_task(void* context) {
    MifareNestedWorker* mifare_nested_worker = context;

    if(mifare_nested_worker->state == MifareNestedWorkerStateCheck) {
        mifare_nested_worker_check(mifare_nested_worker);
    } else if(mifare_nested_worker->state == MifareNestedWorkerStateCollectingStatic) {
        mifare_nested_worker_collect_nonces_static(mifare_nested_worker);
    } else if(mifare_nested_worker->state == MifareNestedWorkerStateCollecting) {
        mifare_nested_worker_collect_nonces(mifare_nested_worker);
    } else if(mifare_nested_worker->state == MifareNestedWorkerStateCollectingHard) {
        mifare_nested_worker_collect_nonces_hard(mifare_nested_worker);
    } else if(mifare_nested_worker->state == MifareNestedWorkerStateValidating) {
        mifare_nested_worker_check_keys(mifare_nested_worker);
    }

    mifare_nested_worker_change_state(mifare_nested_worker, MifareNestedWorkerStateReady);

    return 0;
}

void mifare_nested_worker_write_uid_string(FuriHalNfcDevData* data, FuriString* string) {
    uint8_t* uid = data->uid;
    uint8_t uid_len = data->uid_len;

    for(size_t i = 0; i < uid_len; i++) {
        uint8_t uid_part = uid[i];
        furi_string_cat_printf(string, "%02X", uid_part);
    }
}

void mifare_nested_worker_get_key_cache_file_path(FuriHalNfcDevData* data, FuriString* file_path) {
    furi_string_set(file_path, EXT_PATH("nfc/.cache") "/");

    mifare_nested_worker_write_uid_string(data, file_path);

    furi_string_cat_printf(file_path, ".keys");
}

void mifare_nested_worker_get_nonces_file_path(FuriHalNfcDevData* data, FuriString* file_path) {
    furi_string_set(file_path, NESTED_FOLDER "/");

    mifare_nested_worker_write_uid_string(data, file_path);

    furi_string_cat_printf(file_path, ".nonces");
}

void mifare_nested_worker_get_found_keys_file_path(FuriHalNfcDevData* data, FuriString* file_path) {
    furi_string_set(file_path, NESTED_FOLDER "/");

    mifare_nested_worker_write_uid_string(data, file_path);

    furi_string_cat_printf(file_path, ".keys");
}

void mifare_nested_worker_get_hardnested_folder_path(
    FuriHalNfcDevData* data,
    FuriString* file_path) {
    furi_string_set(file_path, NESTED_FOLDER "/");

    mifare_nested_worker_write_uid_string(data, file_path);
}

void mifare_nested_worker_get_hardnested_file_path(
    FuriHalNfcDevData* data,
    FuriString* file_path,
    uint8_t sector,
    uint8_t key_type) {
    mifare_nested_worker_get_hardnested_folder_path(data, file_path);

    furi_string_cat_printf(file_path, "/%u_%u.nonces", sector, key_type);
}

uint8_t mifare_nested_worker_get_block_by_sector(uint8_t sector) {
    furi_assert(sector < 40);
    if(sector < 32) {
        return (sector * 4) + 3;
    } else {
        return 32 * 4 + (sector - 32) * 16 + 15;
    }
}

static MfClassicSectorTrailer*
    mifare_nested_worker_get_sector_trailer_by_sector(MfClassicData* data, uint8_t sector) {
    return (MfClassicSectorTrailer*)data->block[mifare_nested_worker_get_block_by_sector(sector)]
        .value;
}

bool mifare_nested_worker_read_key_cache(FuriHalNfcDevData* data, MfClassicData* mf_data) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* temp_str = furi_string_alloc();
    mifare_nested_worker_get_key_cache_file_path(data, temp_str);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool load_success = false;
    uint32_t sector_count = 0;

    do {
        if(storage_common_stat(storage, furi_string_get_cstr(temp_str), NULL) != FSE_OK) break;

        if(!flipper_format_file_open_existing(file, furi_string_get_cstr(temp_str))) break;

        uint32_t version = 0;

        if(!flipper_format_read_header(file, temp_str, &version)) break;
        if(furi_string_cmp_str(temp_str, "Flipper NFC keys")) break;

        if(version != 1) break;

        if(!flipper_format_read_string(file, "Mifare Classic type", temp_str)) break;

        if(!furi_string_cmp(temp_str, "1K")) {
            mf_data->type = MfClassicType1k;
            sector_count = 16;
        } else if(!furi_string_cmp(temp_str, "4K")) {
            mf_data->type = MfClassicType4k;
            sector_count = 40;
        } else if(!furi_string_cmp(temp_str, "MINI")) {
            mf_data->type = MfClassicTypeMini;
            sector_count = 5;
        } else {
            break;
        }

        if(!flipper_format_read_hex_uint64(file, "Key A map", &mf_data->key_a_mask, 1)) break;
        if(!flipper_format_read_hex_uint64(file, "Key B map", &mf_data->key_b_mask, 1)) break;

        bool key_read_success = true;

        for(size_t i = 0; (i < sector_count) && (key_read_success); i++) {
            MfClassicSectorTrailer* sec_tr =
                mifare_nested_worker_get_sector_trailer_by_sector(mf_data, i);

            if(FURI_BIT(mf_data->key_a_mask, i)) {
                furi_string_printf(temp_str, "Key A sector %d", i);
                key_read_success = flipper_format_read_hex(
                    file, furi_string_get_cstr(temp_str), sec_tr->key_a, 6);
            }

            if(!key_read_success) break;

            if(FURI_BIT(mf_data->key_b_mask, i)) {
                furi_string_printf(temp_str, "Key B sector %d", i);
                key_read_success = flipper_format_read_hex(
                    file, furi_string_get_cstr(temp_str), sec_tr->key_b, 6);
            }
        }

        load_success = key_read_success;
    } while(false);

    furi_string_free(temp_str);
    flipper_format_free(file);

    return load_success;
}

bool hex_char_to_hex_nibble(char c, uint8_t* nibble) {
    if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
        if(c <= '9') {
            *nibble = c - '0';
        } else if(c <= 'F') {
            *nibble = c - 'A' + 10;
        } else {
            *nibble = c - 'a' + 10;
        }
        return true;
    } else {
        return false;
    }
}

bool hex_char_to_uint8(char hi, char low, uint8_t* value) {
    uint8_t hi_nibble_value, low_nibble_value;

    if(hex_char_to_hex_nibble(hi, &hi_nibble_value) &&
       hex_char_to_hex_nibble(low, &low_nibble_value)) {
        *value = (hi_nibble_value << 4) | low_nibble_value;
        return true;
    } else {
        return false;
    }
}

void free_nonces(NonceList_t* nonces, uint8_t sector_count, uint8_t tries_count) {
    for(uint8_t sector = 0; sector < sector_count; sector++) {
        for(uint8_t key_type = 0; key_type < 2; key_type++) {
            for(uint8_t tries = 0; tries < tries_count; tries++) {
                free(nonces->nonces[sector][key_type][tries]);
            }
        }
    }
}

MfClassicType mifare_nested_worker_get_tag_type(uint8_t ATQA0, uint8_t ATQA1, uint8_t SAK) {
    UNUSED(ATQA1);
    if((ATQA0 == 0x44 || ATQA0 == 0x04)) {
        if((SAK == 0x08 || SAK == 0x88)) {
            return MfClassicType1k;
        } else if(SAK == 0x09) {
            return MfClassicTypeMini;
        }
    } else if((ATQA0 == 0x01) && (ATQA1 == 0x0F) && (SAK == 0x01)) {
        //skylanders support
        return MfClassicType1k;
    } else if((ATQA0 == 0x42 || ATQA0 == 0x02) && (SAK == 0x18)) {
        return MfClassicType4k;
    }
    return MfClassicType1k;
}

uint32_t mifare_nested_worker_predict_delay(
    FuriHalNfcTxRxContext* tx_rx,
    uint8_t blockNo,
    uint8_t keyType,
    uint64_t ui64Key,
    uint32_t tries,
    MifareNestedWorker* mifare_nested_worker) {
    uint32_t cuid = 0;
    Crypto1* crypto = malloc(sizeof(Crypto1));
    uint32_t nt1, nt2, i = 0, previous = 0, prng_delay = 0, zero_prng_value = 65565, repeat = 0;

    if(tries > 25) {
        free(crypto);
        return 2; // Too many tries, fallback to hardnested
    }

    // This part of attack is my attempt to implement it on Flipper.
    // Check README.md for more info

    // First, we find RPNG rounds per 1000 us
    for(uint32_t rtr = 0; rtr < 25; rtr++) {
        if(mifare_nested_worker->state != MifareNestedWorkerStateCollecting) {
            free(crypto);
            return 1;
        }

        nfc_activate();
        if(!furi_hal_nfc_activate_nfca(200, &cuid)) break;

        mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt1);

        furi_delay_us(rtr * 1000);

        mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, true, &nt2);

        // Searching for delay, where PRNG will be near 800
        uint32_t nttmp = prng_successor(nt1, 100);

        for(i = 101; i < 65565; i++) {
            nttmp = prng_successor(nttmp, 1);
            if(nttmp == nt2) break;
        }

        if(!rtr) {
            zero_prng_value = i;
        }

        if(previous && i > previous && i != 65565) {
            if(!prng_delay) {
                prng_delay = i - previous;
            } else if(prng_delay - 100 > i - previous && prng_delay + 100 < i - previous) {
                prng_delay += i - previous;
                prng_delay /= 2;
            }
        }

        previous = i;

        FURI_LOG_D(TAG, "Calibrating: ntdist=%lu, delay=%lu", i, rtr * 1000);

        // Let's hope...
        if(i > 810 && i < 840) {
            free(crypto);
            return rtr * 1000;
        }
    }

    FURI_LOG_D(TAG, "PRNG timing: growth ratio per 1000 us = %lu", prng_delay);

    // Next, we try to calculate time until PRNG near 800 with more perfect timing
    // Mifare Classic (weak) RPNG repeats every 65565 PRNG cycles

    if(zero_prng_value == 65565) {
        free(crypto);
        // PRNG isn't pretictable
        return 1;
    }

    uint32_t cycles_to_reset = (65565 - zero_prng_value) / prng_delay;

    uint32_t limit = 7;

    for(uint32_t rtr = cycles_to_reset - 1; rtr < cycles_to_reset + limit; rtr++) {
        for(uint32_t rtz = 0; rtz < 100; rtz++) {
            if(mifare_nested_worker->state != MifareNestedWorkerStateCollecting) {
                free(crypto);
                return 1;
            }

            nfc_activate();
            if(!furi_hal_nfc_activate_nfca(200, &cuid)) break;

            uint32_t delay = rtr * 1000 + rtz * 10;

            mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, false, &nt1);

            furi_delay_us(delay);

            mifare_classic_authex(crypto, tx_rx, cuid, blockNo, keyType, ui64Key, true, &nt2);

            // Searching for delay, where PRNG will be near 800
            uint32_t nttmp = prng_successor(nt1, 0);

            for(i = 1; i < 65565; i++) {
                nttmp = prng_successor(nttmp, 1);
                if(nttmp == nt2) break;
            }

            if(!(i > previous - 50 && i < previous + 50) && rtz) {
                repeat++;

                if(repeat < 5) {
                    FURI_LOG_D(TAG, "Invalid RPNG value: ntdist=%lu", i);

                    continue;
                }
            }

            if(i > 2000 && i < 65500) {
                uint32_t catch_cycles = (65565 - i) / prng_delay;
                if(catch_cycles > 2) {
                    catch_cycles++;

                    FURI_LOG_D(
                        TAG,
                        "Trying a more accurate value: skipping additional %lu us",
                        catch_cycles * 1000);
                    limit += catch_cycles + 2;
                    rtr += catch_cycles;
                }
            }

            FURI_LOG_D(
                TAG,
                "Calibrating: ntdist=%lu, delay=%lu, max=%lu",
                i,
                delay,
                (cycles_to_reset + limit) * 1000);

            repeat = 0;
            previous = i;

            if(i > 810 && i < 840) {
                free(crypto);
                FURI_LOG_I(TAG, "Found delay: %lu us", delay);
                return delay;
            } else if(i > 840 && i < 40000) {
                FURI_LOG_D(TAG, "Trying again: timing lost");
                tries++;
                free(crypto);
                return mifare_nested_worker_predict_delay(
                    tx_rx, blockNo, keyType, ui64Key, tries, mifare_nested_worker);
            }
        }
    }

    if(i > 1000 && i < 65000) {
        FURI_LOG_D(TAG, "Trying again: wrong predicted timing");
        tries++;
        free(crypto);
        return mifare_nested_worker_predict_delay(
            tx_rx, blockNo, keyType, ui64Key, tries, mifare_nested_worker);
    }

    free(crypto);

    return 1;
}

SaveNoncesResult_t* mifare_nested_worker_write_nonces(
    FuriHalNfcDevData* data,
    Storage* storage,
    NonceList_t* nonces,
    uint8_t tries_count,
    uint8_t free_tries_count,
    uint8_t sector_count,
    uint32_t delay,
    uint32_t distance) {
    FuriString* path = furi_string_alloc();
    Stream* file_stream = file_stream_alloc(storage);
    SaveNoncesResult_t* result = malloc(sizeof(SaveNoncesResult_t));
    result->saved = 0;
    result->invalid = 0;
    result->skipped = 0;

    mifare_nested_worker_get_nonces_file_path(data, path);

    file_stream_open(file_stream, furi_string_get_cstr(path), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS);

    FuriString* header = furi_string_alloc_printf(
        "Filetype: Flipper Nested Nonce Manifest File\nVersion: %s\nNote: you will need desktop app to recover keys: %s\n",
        NESTED_NONCE_FORMAT_VERSION,
        NESTED_RECOVER_KEYS_GITHUB_LINK);
    stream_write_string(file_stream, header);

    for(uint8_t tries = 0; tries < tries_count; tries++) {
        for(uint8_t sector = 0; sector < sector_count; sector++) {
            for(uint8_t key_type = 0; key_type < 2; key_type++) {
                if(nonces->nonces[sector][key_type][tries]->invalid) {
                    if(tries == 0) {
                        result->invalid++;
                    }
                } else if(nonces->nonces[sector][key_type][tries]->skipped) {
                    if(tries == 0) {
                        result->skipped++;
                    }
                } else if(nonces->nonces[sector][key_type][tries]->collected) {
                    if(nonces->nonces[sector][key_type][tries]->hardnested) {
                        FuriString* hardnested_path = furi_string_alloc();
                        mifare_nested_worker_get_hardnested_file_path(
                            data, hardnested_path, sector, key_type);

                        FuriString* str = furi_string_alloc_printf(
                            "HardNested: Key %c cuid 0x%08lx file %s sec %u\n",
                            !key_type ? 'A' : 'B',
                            nonces->cuid,
                            furi_string_get_cstr(hardnested_path),
                            sector);

                        stream_write_string(file_stream, str);

                        furi_string_free(hardnested_path);
                        furi_string_free(str);
                    } else {
                        FuriString* str = furi_string_alloc_printf(
                            "Nested: Key %c cuid 0x%08lx", !key_type ? 'A' : 'B', nonces->cuid);

                        for(uint8_t type = 0; type < 2; type++) {
                            furi_string_cat_printf(
                                str,
                                " nt%u 0x%08lx ks%u 0x%08lx par%u ",
                                type,
                                nonces->nonces[sector][key_type][tries]->target_nt[type],
                                type,
                                nonces->nonces[sector][key_type][tries]->target_ks[type],
                                type);

                            uint8_t* par = nonces->nonces[sector][key_type][tries]->parity[type];
                            for(uint8_t i = 0; i < 4; i++) {
                                furi_string_cat_printf(str, "%u", par[i]);
                            }
                        }

                        furi_string_cat_printf(str, " sec %u\n", sector);

                        stream_write_string(file_stream, str);
                        furi_string_free(str);
                    }

                    result->saved++;
                }
            }
        }
    }

    if(delay) {
        FuriString* str =
            furi_string_alloc_printf("Nested: Delay %lu, distance %lu", delay, distance);

        stream_write_string(file_stream, str);
        furi_string_free(str);
    }

    free_nonces(nonces, sector_count, free_tries_count);
    file_stream_close(file_stream);
    free(file_stream);

    if(!result->saved) {
        FURI_LOG_E(TAG, "No nonces collected, removing file...");
        if(!storage_simply_remove(storage, furi_string_get_cstr(path))) {
            FURI_LOG_E(TAG, "Failed to remove .nonces file");
        }
    }

    furi_string_free(path);
    furi_record_close(RECORD_STORAGE);

    return result;
}

bool mifare_nested_worker_check_initial_keys(
    NonceList_t* nonces,
    MfClassicData* mf_data,
    uint8_t tries_count,
    uint8_t sector_count,
    uint64_t* key,
    uint32_t* key_block,
    uint32_t* found_key_type) {
    bool has_a_key, has_b_key;
    FuriHalNfcTxRxContext tx_rx = {};

    for(uint8_t sector = 0; sector < sector_count; sector++) {
        for(uint8_t key_type = 0; key_type < 2; key_type++) {
            for(uint8_t tries = 0; tries < tries_count; tries++) {
                Nonces* info = malloc(sizeof(Nonces));
                info->key_type = key_type;
                info->block = mifare_nested_worker_get_block_by_sector(sector);
                info->collected = false;
                info->skipped = true;

                nonces->nonces[sector][key_type][tries] = info;
            }
        }
    }

    for(uint8_t sector = 0; sector < sector_count; sector++) {
        MfClassicSectorTrailer* trailer =
            mifare_nested_worker_get_sector_trailer_by_sector(mf_data, sector);
        has_a_key = FURI_BIT(mf_data->key_a_mask, sector);
        has_b_key = FURI_BIT(mf_data->key_b_mask, sector);

        if(has_a_key) {
            for(uint8_t tries = 0; tries < tries_count; tries++) {
                Nonces* info = nonces->nonces[sector][0][tries];
                info->collected = true;
                info->skipped = true;

                nonces->nonces[sector][0][tries] = info;
            }

            if(*key_block == 0) {
                uint64_t key_check = nfc_util_bytes2num(trailer->key_a, 6);
                if(nested_check_key(
                       &tx_rx, mifare_nested_worker_get_block_by_sector(sector), 0, key_check) ==
                   NestedCheckKeyValid) {
                    *key = key_check;
                    *key_block = mifare_nested_worker_get_block_by_sector(sector);
                    *found_key_type = 0;
                }
            }
        }

        if(has_b_key) {
            for(uint8_t tries = 0; tries < tries_count; tries++) {
                Nonces* info = nonces->nonces[sector][1][tries];
                info->collected = true;
                info->skipped = true;

                nonces->nonces[sector][1][tries] = info;
            }

            if(*key_block == 0) {
                uint64_t key_check = nfc_util_bytes2num(trailer->key_b, 6);
                if(nested_check_key(
                       &tx_rx, mifare_nested_worker_get_block_by_sector(sector), 1, key_check) ==
                   NestedCheckKeyValid) {
                    *key = key_check;
                    *key_block = mifare_nested_worker_get_block_by_sector(sector);
                    *found_key_type = 1;
                }
            }
        }
    }

    nonces->cuid = 0;
    nonces->hardnested_states = 0;
    nonces->sector_count = sector_count;
    nonces->tries = tries_count;

    return *key_block;
}

void mifare_nested_worker_check(MifareNestedWorker* mifare_nested_worker) {
    while(mifare_nested_worker->state == MifareNestedWorkerStateCheck) {
        FuriHalNfcTxRxContext tx_rx = {};
        NfcDevice* dev = mifare_nested_worker->context->nfc_dev;
        MfClassicData* mf_data = &dev->dev_data.mf_classic_data;
        FuriHalNfcDevData data = {};
        MifareNestedNonceType type = MifareNestedNonceNoTag;
        nested_get_data(&data);

        if(mifare_nested_worker_read_key_cache(&data, mf_data)) {
            for(uint8_t sector = 0; sector < 40; sector++) {
                if(FURI_BIT(mf_data->key_a_mask, sector) ||
                   FURI_BIT(mf_data->key_b_mask, sector)) {
                    type = nested_check_nonce_type(
                        &tx_rx, mifare_nested_worker_get_block_by_sector(sector));
                    break;
                }
            }

            if(type == MifareNestedNonceNoTag) {
                type = nested_check_nonce_type(&tx_rx, 0);
            }
        } else {
            type = nested_check_nonce_type(&tx_rx, 0);
        }

        if(type == MifareNestedNonceStatic) {
            mifare_nested_worker->context->collecting_type =
                MifareNestedWorkerStateCollectingStatic;

            mifare_nested_worker->callback(
                MifareNestedWorkerEventCollecting, mifare_nested_worker->context);

            break;
        } else if(type == MifareNestedNonceWeak) {
            mifare_nested_worker->context->collecting_type = MifareNestedWorkerStateCollecting;

            mifare_nested_worker->callback(
                MifareNestedWorkerEventCollecting, mifare_nested_worker->context);

            break;
        } else if(type == MifareNestedNonceHard) {
            mifare_nested_worker->context->collecting_type = MifareNestedWorkerStateCollectingHard;

            mifare_nested_worker->callback(
                MifareNestedWorkerEventCollecting, mifare_nested_worker->context);

            break;
        }

        furi_delay_ms(250);
    }

    nfc_deactivate();
}

void mifare_nested_worker_collect_nonces_static(MifareNestedWorker* mifare_nested_worker) {
    NonceList_t nonces;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    NfcDevice* dev = mifare_nested_worker->context->nfc_dev;
    MfClassicData* mf_data = &dev->dev_data.mf_classic_data;
    FuriString* folder_path = furi_string_alloc();
    FuriHalNfcDevData data = {};
    nested_get_data(&data);
    MfClassicType type = mifare_nested_worker_get_tag_type(data.atqa[0], data.atqa[1], data.sak);
    uint64_t key = 0; // Found key for attack
    uint32_t found_key_type = 0;
    uint32_t key_block = 0;
    uint32_t sector_count = 0;

    FURI_LOG_I(TAG, "Running Static Nested attack");
    FuriString* tag_info = furi_string_alloc_printf("Tag UID: ");
    mifare_nested_worker_write_uid_string(&data, tag_info);
    FURI_LOG_I(TAG, "%s", furi_string_get_cstr(tag_info));
    furi_string_free(tag_info);

    if(type == MfClassicType4k) {
        sector_count = 40;
        FURI_LOG_I(TAG, "Found Mifare Classic 4K tag");
    } else if(type == MfClassicType1k) {
        sector_count = 16;
        FURI_LOG_I(TAG, "Found Mifare Classic 1K tag");
    } else { // if(type == MfClassicTypeMini)
        sector_count = 5;
        FURI_LOG_I(TAG, "Found Mifare Classic Mini tag");
    }

    furi_string_set(folder_path, NESTED_FOLDER);
    storage_common_mkdir(storage, furi_string_get_cstr(folder_path));
    furi_string_free(folder_path);

    if(!mifare_nested_worker_read_key_cache(&data, mf_data) ||
       !mifare_nested_worker_check_initial_keys(
           &nonces, mf_data, 1, sector_count, &key, &key_block, &found_key_type)) {
        mifare_nested_worker->callback(
            MifareNestedWorkerEventNeedKey, mifare_nested_worker->context);
        nfc_deactivate();

        free(mf_data);
        free_nonces(&nonces, sector_count, 1);

        return;
    }

    FURI_LOG_I(
        TAG, "Using %c key for block %lu: %012llX", !found_key_type ? 'A' : 'B', key_block, key);

    while(mifare_nested_worker->state == MifareNestedWorkerStateCollectingStatic) {
        FuriHalNfcTxRxContext tx_rx = {};

        for(uint8_t sector = 0; sector < sector_count; sector++) {
            for(uint8_t key_type = 0; key_type < 2; key_type++) {
                Nonces* info = nonces.nonces[sector][key_type][0];

                if(info->collected) {
                    FURI_LOG_I(
                        TAG,
                        "Skipping sector %u, block %u, key_type: %u as we already have a key",
                        sector,
                        mifare_nested_worker_get_block_by_sector(sector),
                        key_type);

                    info->skipped = true;

                    nonces.nonces[sector][key_type][0] = info;

                    mifare_nested_worker->context->nonces = &nonces;

                    mifare_nested_worker->callback(
                        MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);
                    continue;
                }

                if(!nested_check_block(
                       &tx_rx, mifare_nested_worker_get_block_by_sector(sector), key_type)) {
                    FURI_LOG_E(
                        TAG,
                        "Skipping sector %u, block %u, key_type: %u as we can't auth on it",
                        sector,
                        mifare_nested_worker_get_block_by_sector(sector),
                        key_type);

                    info->invalid = true;

                    nonces.nonces[sector][key_type][0] = info;

                    mifare_nested_worker->context->nonces = &nonces;

                    mifare_nested_worker->callback(
                        MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

                    continue;
                }

                while(!info->collected) {
                    if(mifare_nested_worker->state != MifareNestedWorkerStateCollectingStatic) {
                        break;
                    }

                    struct nonce_info_static result = nested_static_nonce_attack(
                        &tx_rx,
                        key_block,
                        found_key_type,
                        mifare_nested_worker_get_block_by_sector(sector),
                        key_type,
                        key);
                    if(result.full) {
                        FURI_LOG_I(
                            TAG,
                            "Accured nonces for sector %u, block %u, key_type: %u",
                            sector,
                            mifare_nested_worker_get_block_by_sector(sector),
                            key_type);

                        info = nonces.nonces[sector][key_type][0];
                        info->collected = true;
                        info->skipped = false;

                        memcpy(&info->target_nt, result.target_nt, sizeof(result.target_nt));
                        memcpy(&info->target_ks, result.target_ks, sizeof(result.target_ks));

                        nonces.nonces[sector][key_type][0] = info;
                        nonces.cuid = result.cuid;
                        nonces.sector_count = sector_count;

                        mifare_nested_worker->context->nonces = &nonces;

                        mifare_nested_worker->callback(
                            MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);
                        break;
                    } else {
                        mifare_nested_worker->callback(
                            MifareNestedWorkerEventNoTagDetected, mifare_nested_worker->context);
                    }
                }
            }
        }

        break;
    }

    SaveNoncesResult_t* result =
        mifare_nested_worker_write_nonces(&data, storage, &nonces, 1, 1, sector_count, 0, 0);

    free(mf_data);

    if(result->saved) {
        mifare_nested_worker->callback(
            MifareNestedWorkerEventNoncesCollected, mifare_nested_worker->context);
    } else {
        mifare_nested_worker->context->save_state = result;

        mifare_nested_worker->callback(
            MifareNestedWorkerEventNoNoncesCollected, mifare_nested_worker->context);
    }

    nfc_deactivate();
}

void mifare_nested_worker_collect_nonces_hard(MifareNestedWorker* mifare_nested_worker) {
    NonceList_t nonces;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    NfcDevice* dev = mifare_nested_worker->context->nfc_dev;
    MfClassicData* mf_data = &dev->dev_data.mf_classic_data;
    FuriString* folder_path = furi_string_alloc();
    FuriHalNfcDevData data = {};
    nested_get_data(&data);
    MfClassicType type = mifare_nested_worker_get_tag_type(data.atqa[0], data.atqa[1], data.sak);
    uint64_t key = 0; // Found key for attack
    uint32_t found_key_type = 0;
    uint32_t key_block = 0;
    uint32_t sector_count = 0;
    uint32_t cuid = 0;
    furi_hal_nfc_activate_nfca(200, &cuid);

    FURI_LOG_I(TAG, "Running Hard Nested attack");
    FuriString* tag_info = furi_string_alloc_printf("Tag UID: ");
    mifare_nested_worker_write_uid_string(&data, tag_info);
    FURI_LOG_I(TAG, "%s", furi_string_get_cstr(tag_info));
    furi_string_free(tag_info);

    if(type == MfClassicType4k) {
        sector_count = 40;
        FURI_LOG_I(TAG, "Found Mifare Classic 4K tag");
    } else if(type == MfClassicType1k) {
        sector_count = 16;
        FURI_LOG_I(TAG, "Found Mifare Classic 1K tag");
    } else { // if(type == MfClassicTypeMini)
        sector_count = 5;
        FURI_LOG_I(TAG, "Found Mifare Classic Mini tag");
    }

    furi_string_set(folder_path, NESTED_FOLDER);
    storage_common_mkdir(storage, furi_string_get_cstr(folder_path));
    mifare_nested_worker_get_hardnested_folder_path(&data, folder_path);
    storage_common_mkdir(storage, furi_string_get_cstr(folder_path));
    furi_string_free(folder_path);

    if(!mifare_nested_worker_read_key_cache(&data, mf_data) ||
       !mifare_nested_worker_check_initial_keys(
           &nonces, mf_data, 1, sector_count, &key, &key_block, &found_key_type)) {
        mifare_nested_worker->callback(
            MifareNestedWorkerEventNeedKey, mifare_nested_worker->context);
        nfc_deactivate();

        free(mf_data);
        free_nonces(&nonces, sector_count, 1);

        return;
    }

    FURI_LOG_I(
        TAG, "Using %c key for block %lu: %012llX", !found_key_type ? 'A' : 'B', key_block, key);

    FuriHalNfcTxRxContext tx_rx = {};
    nonces.tries = 1;
    nonces.hardnested_states = 0;
    nonces.sector_count = sector_count;

    mifare_nested_worker->context->nonces = &nonces;

    mifare_nested_worker->callback(MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

    mifare_nested_worker->callback(
        MifareNestedWorkerEventHardnestedStatesFound, mifare_nested_worker->context);

    for(uint8_t sector = 0; sector < sector_count &&
                            mifare_nested_worker->state == MifareNestedWorkerStateCollectingHard;
        sector++) {
        for(uint8_t key_type = 0;
            key_type < 2 && mifare_nested_worker->state == MifareNestedWorkerStateCollectingHard;
            key_type++) {
            Nonces* info = nonces.nonces[sector][key_type][0];
            if(info->collected) {
                FURI_LOG_I(
                    TAG,
                    "Skipping sector %u, block %u, key_type: %u as we already have a key",
                    sector,
                    mifare_nested_worker_get_block_by_sector(sector),
                    key_type);

                info->skipped = true;

                nonces.nonces[sector][key_type][0] = info;
                mifare_nested_worker->context->nonces = &nonces;

                mifare_nested_worker->callback(
                    MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

                continue;
            }

            if(!nested_check_block(
                   &tx_rx, mifare_nested_worker_get_block_by_sector(sector), key_type)) {
                FURI_LOG_E(
                    TAG,
                    "Skipping sector %u, block %u, key_type: %u as we can't auth on it",
                    sector,
                    mifare_nested_worker_get_block_by_sector(sector),
                    key_type);

                info->invalid = true;

                nonces.nonces[sector][key_type][0] = info;
                mifare_nested_worker->context->nonces = &nonces;

                mifare_nested_worker->callback(
                    MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

                continue;
            }

            while(!info->collected &&
                  mifare_nested_worker->state == MifareNestedWorkerStateCollectingHard) {
                Stream* file_stream = file_stream_alloc(storage);
                FuriString* hardnested_file = furi_string_alloc();
                mifare_nested_worker_get_hardnested_file_path(
                    &data, hardnested_file, sector, key_type);

                file_stream_open(
                    file_stream,
                    furi_string_get_cstr(hardnested_file),
                    FSAM_READ_WRITE,
                    FSOM_CREATE_ALWAYS);

                FuriString* header = furi_string_alloc_printf(
                    "Filetype: Flipper Nested Nonces File\nVersion: %s\nNote: you will need desktop app to recover keys: %s\nKey %c cuid 0x%08lx sec %u\n",
                    NESTED_NONCE_FORMAT_VERSION,
                    NESTED_RECOVER_KEYS_GITHUB_LINK,
                    !key_type ? 'A' : 'B',
                    cuid,
                    sector);

                stream_write_string(file_stream, header);
                furi_string_free(header);

                uint32_t first_byte_sum = 0;
                uint32_t* found = malloc(sizeof(uint32_t) * 256);
                for(uint32_t i = 0; i < 256; i++) {
                    found[i] = 0;
                }

                while(mifare_nested_worker->state == MifareNestedWorkerStateCollectingHard) {
                    struct nonce_info_hard result = nested_hard_nonce_attack(
                        &tx_rx,
                        key_block,
                        found_key_type,
                        mifare_nested_worker_get_block_by_sector(sector),
                        key_type,
                        key,
                        found,
                        &first_byte_sum,
                        file_stream);

                    if(result.static_encrypted) {
                        file_stream_close(file_stream);

                        storage_simply_remove(storage, furi_string_get_cstr(hardnested_file));

                        furi_string_free(hardnested_file);
                        free(found);
                        free(mf_data);
                        nfc_deactivate();

                        mifare_nested_worker->callback(
                            MifareNestedWorkerEventStaticEncryptedNonce,
                            mifare_nested_worker->context);

                        return;
                    }

                    if(result.full) {
                        uint32_t states = 0;
                        for(uint32_t i = 0; i < 256; i++) {
                            states += found[i];
                        }

                        nonces.hardnested_states = states;

                        mifare_nested_worker->callback(
                            MifareNestedWorkerEventHardnestedStatesFound,
                            mifare_nested_worker->context);

                        FURI_LOG_D(TAG, "Found states: %lu", states);

                        if(states == 256) {
                            FURI_LOG_D(
                                TAG, "All states collected, first_byte_sum: %lu", first_byte_sum);

                            bool valid = false;
                            for(uint8_t i = 0; i < sizeof(sums); i++) {
                                if(sums[i] == first_byte_sum) {
                                    valid = true;
                                    break;
                                }
                            }

                            if(!valid) {
                                FURI_LOG_E(TAG, "Invalid first_byte_sum!");
                                break;
                            }

                            info->collected = true;
                            info->hardnested = true;
                            info->skipped = false;

                            nonces.cuid = result.cuid;

                            nonces.nonces[sector][key_type][0] = info;

                            mifare_nested_worker->context->nonces = &nonces;

                            mifare_nested_worker->callback(
                                MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

                            break;
                        }
                    } else {
                        mifare_nested_worker->callback(
                            MifareNestedWorkerEventNoTagDetected, mifare_nested_worker->context);
                    }
                }

                free(found);
                furi_string_free(hardnested_file);
                file_stream_close(file_stream);
            }
        }
    }

    SaveNoncesResult_t* result =
        mifare_nested_worker_write_nonces(&data, storage, &nonces, 1, 1, sector_count, 0, 0);

    free(mf_data);

    if(result->saved) {
        mifare_nested_worker->callback(
            MifareNestedWorkerEventNoncesCollected, mifare_nested_worker->context);
    } else {
        mifare_nested_worker->context->save_state = result;

        mifare_nested_worker->callback(
            MifareNestedWorkerEventNoNoncesCollected, mifare_nested_worker->context);
    }

    nfc_deactivate();
}

void mifare_nested_worker_collect_nonces(MifareNestedWorker* mifare_nested_worker) {
    NonceList_t nonces;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    NfcDevice* dev = mifare_nested_worker->context->nfc_dev;
    MfClassicData* mf_data = &dev->dev_data.mf_classic_data;
    FuriString* folder_path = furi_string_alloc();
    FuriHalNfcDevData data = {};
    nested_get_data(&data);
    MfClassicType type = mifare_nested_worker_get_tag_type(data.atqa[0], data.atqa[1], data.sak);
    uint64_t key = 0; // Found key for attack
    uint32_t found_key_type = 0;
    uint32_t key_block = 0;
    uint32_t sector_count = 0;
    uint32_t delay = 0;
    uint32_t distance = 0;
    uint32_t tries_count = 1;

    FURI_LOG_I(TAG, "Running Nested attack");
    FuriString* tag_info = furi_string_alloc_printf("Tag UID: ");
    mifare_nested_worker_write_uid_string(&data, tag_info);
    FURI_LOG_I(TAG, "%s", furi_string_get_cstr(tag_info));
    furi_string_free(tag_info);

    if(type == MfClassicType4k) {
        sector_count = 40;
        FURI_LOG_I(TAG, "Found Mifare Classic 4K tag");
    } else if(type == MfClassicType1k) {
        sector_count = 16;
        FURI_LOG_I(TAG, "Found Mifare Classic 1K tag");
    } else { // if(type == MfClassicTypeMini)
        sector_count = 5;
        FURI_LOG_I(TAG, "Found Mifare Classic Mini tag");
    }

    furi_string_set(folder_path, NESTED_FOLDER);
    storage_common_mkdir(storage, furi_string_get_cstr(folder_path));
    furi_string_free(folder_path);

    if(!mifare_nested_worker_read_key_cache(&data, mf_data) ||
       !mifare_nested_worker_check_initial_keys(
           &nonces, mf_data, 3, sector_count, &key, &key_block, &found_key_type)) {
        mifare_nested_worker->callback(
            MifareNestedWorkerEventNeedKey, mifare_nested_worker->context);
        nfc_deactivate();

        free(mf_data);
        free_nonces(&nonces, sector_count, 3);

        return;
    }

    FURI_LOG_I(
        TAG, "Using %c key for block %lu: %012llX", !found_key_type ? 'A' : 'B', key_block, key);

    while(mifare_nested_worker->state == MifareNestedWorkerStateCollecting) {
        FuriHalNfcTxRxContext tx_rx = {};
        uint32_t first_distance = 0;
        uint32_t second_distance = 0;

        mifare_nested_worker->callback(
            MifareNestedWorkerEventCalibrating, mifare_nested_worker->context);

        distance = nested_calibrate_distance(&tx_rx, key_block, found_key_type, key, delay, false);

        if(mifare_nested_worker->state == MifareNestedWorkerStateCollecting) {
            first_distance =
                nested_calibrate_distance(&tx_rx, key_block, found_key_type, key, delay, true);
        }

        if(mifare_nested_worker->state == MifareNestedWorkerStateCollecting) {
            second_distance =
                nested_calibrate_distance(&tx_rx, key_block, found_key_type, key, 10000, true);
        }

        if(first_distance == 0 && second_distance == 0) {
            nfc_deactivate();

            free(mf_data);
            free_nonces(&nonces, sector_count, 3);

            mifare_nested_worker_change_state(
                mifare_nested_worker, MifareNestedWorkerStateCollectingHard);

            mifare_nested_worker_collect_nonces_hard(mifare_nested_worker);
            return;
        }

        if(first_distance < second_distance - 100 && second_distance > 100) {
            FURI_LOG_E(
                TAG,
                "Discovered tag with PRNG that depends on time. PRNG values: %lu, %lu",
                first_distance,
                second_distance);

            struct distance_info info =
                nested_calibrate_distance_info(&tx_rx, key_block, found_key_type, key);

            if(info.max_prng - info.min_prng > 150) {
                FURI_LOG_W(
                    TAG,
                    "PRNG is too unpredictable (min/max values more than 150: %lu - %lu = %lu), fallback to delay method",
                    info.max_prng,
                    info.min_prng,
                    info.max_prng - info.min_prng);

                delay = 1;
            } else {
                FURI_LOG_I(
                    TAG,
                    "PRNG is stable, using method without delay! (May be false positive, still will collect x3 times)");

                distance =
                    nested_calibrate_distance(&tx_rx, key_block, found_key_type, key, delay, true);

                delay = 2;
                tries_count = 3;
            }
        }

        if(distance == 0 || delay == 1) {
            bool failed = false;
            // Tag need delay or unpredictable PRNG
            FURI_LOG_W(TAG, "Can't determine distance, trying to find timing...");

            mifare_nested_worker->callback(
                MifareNestedWorkerEventNeedPrediction, mifare_nested_worker->context);

            delay = mifare_nested_worker_predict_delay(
                &tx_rx, key_block, found_key_type, key, 0, mifare_nested_worker);

            if(delay == 1) {
                FURI_LOG_E(TAG, "Can't determine delay");

                // Check that we didn't lost tag
                FuriHalNfcDevData lost_tag_data = {};
                nested_get_data(&lost_tag_data);
                if(lost_tag_data.uid_len == 0) {
                    // We lost it.
                    mifare_nested_worker->callback(
                        MifareNestedWorkerEventNoTagDetected, mifare_nested_worker->context);

                    while(mifare_nested_worker->state == MifareNestedWorkerStateCollecting &&
                          lost_tag_data.cuid != data.cuid) {
                        furi_delay_ms(250);
                        nested_get_data(&lost_tag_data);
                    }

                    mifare_nested_worker->callback(
                        MifareNestedWorkerEventCalibrating, mifare_nested_worker->context);

                    continue;
                }

                failed = true;
            }

            if(delay == 2) {
                FURI_LOG_E(TAG, "Can't determine delay in 25 tries, fallback to hardnested");

                nfc_deactivate();

                free(mf_data);
                free_nonces(&nonces, sector_count, 3);

                mifare_nested_worker_change_state(
                    mifare_nested_worker, MifareNestedWorkerStateCollectingHard);

                mifare_nested_worker_collect_nonces_hard(mifare_nested_worker);
                return;
            }

            if(mifare_nested_worker->state == MifareNestedWorkerStateCollecting && !failed) {
                distance = nested_calibrate_distance(
                    &tx_rx, key_block, found_key_type, key, delay, false);
            }

            if(distance == 0 && !failed) {
                FURI_LOG_E(TAG, "Found delay, but can't find distance");

                failed = true;
            }

            if(failed) {
                nfc_deactivate();

                mifare_nested_worker->callback(
                    MifareNestedWorkerEventAttackFailed, mifare_nested_worker->context);

                free(mf_data);
                free_nonces(&nonces, sector_count, 3);

                return;
            }

            tries_count = 3;
        }

        mifare_nested_worker->context->nonces = &nonces;

        mifare_nested_worker->callback(
            MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

        for(uint8_t tries = 0; tries < tries_count; tries++) {
            for(uint8_t sector = 0; sector < sector_count; sector++) {
                for(uint8_t key_type = 0; key_type < 2; key_type++) {
                    Nonces* info = nonces.nonces[sector][key_type][tries];
                    if(info->collected) {
                        FURI_LOG_I(
                            TAG,
                            "Skipping sector %u, block %u, key_type: %u as we already have a key",
                            sector,
                            mifare_nested_worker_get_block_by_sector(sector),
                            key_type);

                        info->skipped = true;

                        nonces.nonces[sector][key_type][tries] = info;
                        mifare_nested_worker->context->nonces = &nonces;

                        mifare_nested_worker->callback(
                            MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

                        continue;
                    }

                    if(!nested_check_block(
                           &tx_rx, mifare_nested_worker_get_block_by_sector(sector), key_type)) {
                        FURI_LOG_E(
                            TAG,
                            "Skipping sector %u, block %u, key_type: %u as we can't auth on it",
                            sector,
                            mifare_nested_worker_get_block_by_sector(sector),
                            key_type);

                        info->skipped = true;

                        nonces.nonces[sector][key_type][0] = info;
                        mifare_nested_worker->context->nonces = &nonces;

                        mifare_nested_worker->callback(
                            MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);

                        continue;
                    }

                    while(!info->collected) {
                        if(mifare_nested_worker->state != MifareNestedWorkerStateCollecting) {
                            break;
                        }

                        struct nonce_info result = nested_attack(
                            &tx_rx,
                            key_block,
                            found_key_type,
                            mifare_nested_worker_get_block_by_sector(sector),
                            key_type,
                            key,
                            distance,
                            delay);

                        if(result.full) {
                            FURI_LOG_I(
                                TAG,
                                "Accured nonces for sector %u, block %u, key_type: %u",
                                sector,
                                mifare_nested_worker_get_block_by_sector(sector),
                                key_type);

                            info = nonces.nonces[sector][key_type][tries];
                            info->collected = true;
                            info->skipped = false;

                            memcpy(&info->target_nt, result.target_nt, sizeof(result.target_nt));
                            memcpy(&info->target_ks, result.target_ks, sizeof(result.target_ks));
                            memcpy(&info->parity, result.parity, sizeof(result.parity));

                            nonces.nonces[sector][key_type][tries] = info;
                            nonces.cuid = result.cuid;
                            nonces.sector_count = sector_count;
                            nonces.tries = tries_count;

                            mifare_nested_worker->context->nonces = &nonces;

                            mifare_nested_worker->callback(
                                MifareNestedWorkerEventNewNonce, mifare_nested_worker->context);
                            break;
                        } else {
                            mifare_nested_worker->callback(
                                MifareNestedWorkerEventNoTagDetected,
                                mifare_nested_worker->context);
                        }
                    }
                }
            }
        }

        break;
    }

    SaveNoncesResult_t* result = mifare_nested_worker_write_nonces(
        &data, storage, &nonces, tries_count, 3, sector_count, delay, distance);

    free(mf_data);

    if(result->saved) {
        mifare_nested_worker->callback(
            MifareNestedWorkerEventNoncesCollected, mifare_nested_worker->context);
    } else {
        mifare_nested_worker->context->save_state = result;

        mifare_nested_worker->callback(
            MifareNestedWorkerEventNoNoncesCollected, mifare_nested_worker->context);
    }

    nfc_deactivate();
}

bool* mifare_nested_worker_check_keys_exists(
    Storage* storage,
    char* path,
    uint64_t* keys,
    uint32_t key_count,
    MifareNestedWorker* mifare_nested_worker) {
    bool* old_keys = malloc(sizeof(bool) * key_count);
    Stream* file_stream = file_stream_alloc(storage);
    file_stream_open(file_stream, path, FSAM_READ, FSOM_OPEN_ALWAYS);
    FuriString* key_strings[key_count];

    for(uint32_t i = 0; i < key_count; i++) {
        old_keys[i] = false;
        key_strings[i] = furi_string_alloc_printf("%012llX\n", keys[i]);
    }

    while(mifare_nested_worker->state == MifareNestedWorkerStateValidating) {
        FuriString* next_line = furi_string_alloc();

        if(!stream_read_line(file_stream, next_line)) {
            break;
        }

        for(uint32_t i = 0; i < key_count; i++) {
            if(keys[i] == (uint64_t)-1) continue;

            if(furi_string_cmp(next_line, key_strings[i]) == 0) {
                old_keys[i] = true;
            }
        }

        furi_string_free(next_line);
    }

    for(uint32_t i = 0; i < key_count; i++) {
        furi_string_free(key_strings[i]);
    }

    file_stream_close(file_stream);
    free(file_stream);

    return old_keys;
}

void mifare_nested_worker_write_key(Storage* storage, FuriString* key) {
    Stream* file_stream = file_stream_alloc(storage);
    file_stream_open(
        file_stream,
        EXT_PATH("nfc/assets/mf_classic_dict_user.nfc"),
        FSAM_READ_WRITE,
        FSOM_OPEN_APPEND);

    stream_write_string(file_stream, key);

    file_stream_close(file_stream);
}

void mifare_nested_worker_check_keys(MifareNestedWorker* mifare_nested_worker) {
    KeyInfo_t* key_info = mifare_nested_worker->context->keys;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* file_stream = file_stream_alloc(storage);
    FuriString* next_line = furi_string_alloc();
    FuriString* path = furi_string_alloc();
    FuriHalNfcDevData data = {};
    nested_get_data(&data);
    MfClassicType type = mifare_nested_worker_get_tag_type(data.atqa[0], data.atqa[1], data.sak);
    NestedCheckKeyResult result = NestedCheckKeyNoTag;
    FuriHalNfcTxRxContext tx_rx = {};
    uint32_t key_count = 0;
    uint32_t sector_key_count = 0;
    uint64_t keys[80];
    bool found_keys[2][40];
    bool unique_keys[2][40];
    uint32_t sector_count = 0;

    if(type == MfClassicType4k) {
        sector_count = 40;
        FURI_LOG_I(TAG, "Found Mifare Classic 4K tag");
    } else if(type == MfClassicType1k) {
        sector_count = 16;
        FURI_LOG_I(TAG, "Found Mifare Classic 1K tag");
    } else { // if(type == MfClassicTypeMini)
        sector_count = 5;
        FURI_LOG_I(TAG, "Found Mifare Classic Mini tag");
    }

    uint32_t keys_count = sector_count * 2;

    for(uint8_t key = 0; key < 2; key++) {
        for(uint8_t i = 0; i < sector_count; i++) {
            found_keys[key][i] = false;
            unique_keys[key][i] = false;
        }
    }

    for(uint8_t i = 0; i < keys_count; i++) {
        keys[i] = -1;
    }

    mifare_nested_worker_get_found_keys_file_path(&data, path);

    if(!file_stream_open(file_stream, furi_string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Can't open %s", furi_string_get_cstr(path));

        file_stream_close(file_stream);

        mifare_nested_worker_get_nonces_file_path(&data, path);

        if(!file_stream_open(
               file_stream, furi_string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            mifare_nested_worker->callback(
                MifareNestedWorkerEventNeedCollection, mifare_nested_worker->context);
        } else {
            mifare_nested_worker->callback(
                MifareNestedWorkerEventNeedKeyRecovery, mifare_nested_worker->context);
        }

        file_stream_close(file_stream);

        free(file_stream);
        furi_string_free(path);
        furi_string_free(next_line);
        furi_record_close(RECORD_STORAGE);

        return;
    };

    while(true) {
        if(!stream_read_line(file_stream, next_line)) {
            break;
        }

        if(furi_string_start_with_str(next_line, "Key")) {
            uint8_t key_type = furi_string_get_char(next_line, 4) == 'B';
            uint8_t sector = atoi((char[]){furi_string_get_char(next_line, 13)}) * 10 +
                             atoi((char[]){furi_string_get_char(next_line, 14)});

            if(!unique_keys[key_type][sector]) {
                unique_keys[key_type][sector] = true;
                sector_key_count++;
            }
        }

        key_count++;
    }

    stream_rewind(file_stream);

    key_info->total_keys = key_count;
    key_info->sector_keys = sector_key_count;

    while(mifare_nested_worker->state == MifareNestedWorkerStateValidating) {
        if(!stream_read_line(file_stream, next_line)) {
            break;
        }

        if(furi_string_start_with_str(next_line, "Key")) {
            // Key X sector XX: XX XX XX XX XX XX
            // 0000000000111111111122222222223333
            // 0123456789012345678901234567890123
            uint8_t keyChar[6];
            uint8_t count = 0;

            uint8_t key_type = furi_string_get_char(next_line, 4) == 'B';
            uint8_t sector = atoi((char[]){furi_string_get_char(next_line, 13)}) * 10 +
                             atoi((char[]){furi_string_get_char(next_line, 14)});

            for(uint8_t i = 17; i < 33; i += 3) {
                hex_char_to_uint8(
                    furi_string_get_char(next_line, i),
                    furi_string_get_char(next_line, i + 1),
                    &keyChar[count]);
                count++;
            }

            uint64_t key = nfc_util_bytes2num(keyChar, 6);

            key_info->checked_keys++;

            if(found_keys[key_type][sector]) {
                mifare_nested_worker->callback(
                    MifareNestedWorkerEventKeyChecked, mifare_nested_worker->context);

                continue;
            }

            while(mifare_nested_worker->state == MifareNestedWorkerStateValidating) {
                result = nested_check_key(
                    &tx_rx, mifare_nested_worker_get_block_by_sector(sector), key_type, key);

                if(result == NestedCheckKeyNoTag) {
                    mifare_nested_worker->callback(
                        MifareNestedWorkerEventNoTagDetected, mifare_nested_worker->context);

                    furi_delay_ms(250);
                } else {
                    break;
                }
            }

            if(result == NestedCheckKeyValid) {
                FURI_LOG_I(
                    TAG, "Found valid %c key for sector %u: %012llX", key_type, sector, key);
                bool exists = false;

                for(uint8_t i = 0; i < keys_count; i++) {
                    if(keys[i] == key) {
                        exists = true;
                    }
                }

                if(!exists) {
                    keys[key_info->found_keys] = key;
                }

                key_info->found_keys++;
                found_keys[key_type][sector] = true;
            }

            mifare_nested_worker->callback(
                MifareNestedWorkerEventKeyChecked, mifare_nested_worker->context);
        }
    }

    furi_string_free(next_line);
    file_stream_close(file_stream);
    free(file_stream);

    mifare_nested_worker->callback(
        MifareNestedWorkerEventProcessingKeys, mifare_nested_worker->context);

    bool* old_keys = mifare_nested_worker_check_keys_exists(
        storage,
        EXT_PATH("nfc/assets/mf_classic_dict_user.nfc"),
        keys,
        keys_count,
        mifare_nested_worker);

    for(uint8_t i = 0; i < keys_count; i++) {
        if(old_keys[i]) {
            keys[i] = -1;
        }
    }

    old_keys = mifare_nested_worker_check_keys_exists(
        storage,
        EXT_PATH("nfc/assets/mf_classic_dict.nfc"),
        keys,
        keys_count,
        mifare_nested_worker);

    for(uint8_t i = 0; i < keys_count; i++) {
        if(old_keys[i]) {
            keys[i] = -1;
        }
    }

    for(uint8_t i = 0; i < keys_count; i++) {
        if(keys[i] == (uint64_t)-1) continue;

        FuriString* key_string = furi_string_alloc_printf("%012llX\n", keys[i]);

        mifare_nested_worker_write_key(storage, key_string);
        FURI_LOG_I(TAG, "Added new key: %s", furi_string_get_cstr(key_string));

        key_info->added_keys++;

        furi_string_free(key_string);
    }

    if(!storage_simply_remove(storage, furi_string_get_cstr(path))) {
        FURI_LOG_E(TAG, "Failed to remove .keys file");
    }

    furi_record_close(RECORD_STORAGE);
    furi_string_free(path);

    mifare_nested_worker->callback(
        MifareNestedWorkerEventKeysFound, mifare_nested_worker->context);

    return;
}
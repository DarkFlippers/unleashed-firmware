#include "nfc_magic_worker_i.h"

#include "nfc_magic_i.h"
#include "lib/magic/common.h"
#include "lib/magic/classic_gen1.h"
#include "lib/magic/gen4.h"

#define TAG "NfcMagicWorker"

static void
    nfc_magic_worker_change_state(NfcMagicWorker* nfc_magic_worker, NfcMagicWorkerState state) {
    furi_assert(nfc_magic_worker);

    nfc_magic_worker->state = state;
}

NfcMagicWorker* nfc_magic_worker_alloc() {
    NfcMagicWorker* nfc_magic_worker = malloc(sizeof(NfcMagicWorker));

    // Worker thread attributes
    nfc_magic_worker->thread =
        furi_thread_alloc_ex("NfcMagicWorker", 8192, nfc_magic_worker_task, nfc_magic_worker);

    nfc_magic_worker->callback = NULL;
    nfc_magic_worker->context = NULL;

    nfc_magic_worker_change_state(nfc_magic_worker, NfcMagicWorkerStateReady);

    return nfc_magic_worker;
}

void nfc_magic_worker_free(NfcMagicWorker* nfc_magic_worker) {
    furi_assert(nfc_magic_worker);

    furi_thread_free(nfc_magic_worker->thread);
    free(nfc_magic_worker);
}

void nfc_magic_worker_stop(NfcMagicWorker* nfc_magic_worker) {
    furi_assert(nfc_magic_worker);

    nfc_magic_worker_change_state(nfc_magic_worker, NfcMagicWorkerStateStop);
    furi_thread_join(nfc_magic_worker->thread);
}

void nfc_magic_worker_start(
    NfcMagicWorker* nfc_magic_worker,
    NfcMagicWorkerState state,
    NfcMagicDevice* magic_dev,
    NfcDeviceData* dev_data,
    uint32_t new_password,
    NfcMagicWorkerCallback callback,
    void* context) {
    furi_assert(nfc_magic_worker);
    furi_assert(magic_dev);
    furi_assert(dev_data);

    furi_hal_nfc_deinit();
    furi_hal_nfc_init();

    nfc_magic_worker->callback = callback;
    nfc_magic_worker->context = context;
    nfc_magic_worker->magic_dev = magic_dev;
    nfc_magic_worker->dev_data = dev_data;
    nfc_magic_worker->new_password = new_password;
    nfc_magic_worker_change_state(nfc_magic_worker, state);
    furi_thread_start(nfc_magic_worker->thread);
}

int32_t nfc_magic_worker_task(void* context) {
    NfcMagicWorker* nfc_magic_worker = context;

    if(nfc_magic_worker->state == NfcMagicWorkerStateCheck) {
        nfc_magic_worker_check(nfc_magic_worker);
    } else if(nfc_magic_worker->state == NfcMagicWorkerStateWrite) {
        nfc_magic_worker_write(nfc_magic_worker);
    } else if(nfc_magic_worker->state == NfcMagicWorkerStateRekey) {
        nfc_magic_worker_rekey(nfc_magic_worker);
    } else if(nfc_magic_worker->state == NfcMagicWorkerStateWipe) {
        nfc_magic_worker_wipe(nfc_magic_worker);
    }

    nfc_magic_worker_change_state(nfc_magic_worker, NfcMagicWorkerStateReady);

    return 0;
}

void nfc_magic_worker_write(NfcMagicWorker* nfc_magic_worker) {
    bool card_found_notified = false;
    bool done = false;
    FuriHalNfcDevData nfc_data = {};
    NfcMagicDevice* magic_dev = nfc_magic_worker->magic_dev;
    NfcDeviceData* dev_data = nfc_magic_worker->dev_data;
    NfcProtocol dev_protocol = dev_data->protocol;

    while(nfc_magic_worker->state == NfcMagicWorkerStateWrite) {
        do {
            if(magic_dev->type == MagicTypeClassicGen1) {
                if(furi_hal_nfc_detect(&nfc_data, 200)) {
                    magic_deactivate();
                    magic_activate();
                    if(!magic_gen1_wupa()) {
                        FURI_LOG_E(TAG, "No card response to WUPA (not a magic card)");
                        nfc_magic_worker->callback(
                            NfcMagicWorkerEventWrongCard, nfc_magic_worker->context);
                        done = true;
                        break;
                    }
                    magic_deactivate();
                }
                magic_activate();
                if(magic_gen1_wupa()) {
                    magic_gen1_data_access_cmd();

                    MfClassicData* mfc_data = &dev_data->mf_classic_data;
                    for(size_t i = 0; i < 64; i++) {
                        FURI_LOG_D(TAG, "Writing block %d", i);
                        if(!magic_gen1_write_blk(i, &mfc_data->block[i])) {
                            FURI_LOG_E(TAG, "Failed to write %d block", i);
                            done = true;
                            nfc_magic_worker->callback(
                                NfcMagicWorkerEventFail, nfc_magic_worker->context);
                            break;
                        }
                    }

                    done = true;
                    nfc_magic_worker->callback(
                        NfcMagicWorkerEventSuccess, nfc_magic_worker->context);
                    break;
                }
            } else if(magic_dev->type == MagicTypeGen4) {
                if(furi_hal_nfc_detect(&nfc_data, 200)) {
                    uint8_t gen4_config[28];
                    uint32_t password = magic_dev->password;

                    uint32_t cuid;
                    if(dev_protocol == NfcDeviceProtocolMifareClassic) {
                        gen4_config[0] = 0x00;
                        gen4_config[27] = 0x00;
                    } else if(dev_protocol == NfcDeviceProtocolMifareUl) {
                        MfUltralightData* mf_ul_data = &dev_data->mf_ul_data;
                        gen4_config[0] = 0x01;
                        switch(mf_ul_data->type) {
                        case MfUltralightTypeUL11:
                        case MfUltralightTypeUL21:
                        // UL-C?
                        // UL?
                        default:
                            gen4_config[27] = MagicGen4UltralightModeUL_EV1;
                            break;
                        case MfUltralightTypeNTAG203:
                        case MfUltralightTypeNTAG213:
                        case MfUltralightTypeNTAG215:
                        case MfUltralightTypeNTAG216:
                        case MfUltralightTypeNTAGI2C1K:
                        case MfUltralightTypeNTAGI2C2K:
                        case MfUltralightTypeNTAGI2CPlus1K:
                        case MfUltralightTypeNTAGI2CPlus2K:
                            gen4_config[27] = MagicGen4UltralightModeNTAG;
                            break;
                        }
                    }

                    if(dev_data->nfc_data.uid_len == 4) {
                        gen4_config[1] = MagicGen4UIDLengthSingle;
                    } else if(dev_data->nfc_data.uid_len == 7) {
                        gen4_config[1] = MagicGen4UIDLengthDouble;
                    } else {
                        FURI_LOG_E(TAG, "Unexpected UID length %d", dev_data->nfc_data.uid_len);
                        nfc_magic_worker->callback(
                            NfcMagicWorkerEventFail, nfc_magic_worker->context);
                        done = true;
                        break;
                    }

                    gen4_config[2] = (uint8_t)(password >> 24);
                    gen4_config[3] = (uint8_t)(password >> 16);
                    gen4_config[4] = (uint8_t)(password >> 8);
                    gen4_config[5] = (uint8_t)password;

                    if(dev_protocol == NfcDeviceProtocolMifareUl) {
                        gen4_config[6] = MagicGen4ShadowModeHighSpeedIgnore;
                    } else {
                        gen4_config[6] = MagicGen4ShadowModeIgnore;
                    }
                    gen4_config[7] = 0x00;
                    memset(gen4_config + 8, 0, 16);
                    gen4_config[24] = dev_data->nfc_data.atqa[0];
                    gen4_config[25] = dev_data->nfc_data.atqa[1];
                    gen4_config[26] = dev_data->nfc_data.sak;

                    furi_hal_nfc_sleep();
                    furi_hal_nfc_activate_nfca(200, &cuid);
                    if(!magic_gen4_set_cfg(password, gen4_config, sizeof(gen4_config), false)) {
                        nfc_magic_worker->callback(
                            NfcMagicWorkerEventFail, nfc_magic_worker->context);
                        done = true;
                        break;
                    }
                    if(dev_protocol == NfcDeviceProtocolMifareClassic) {
                        MfClassicData* mfc_data = &dev_data->mf_classic_data;
                        size_t block_count = 64;
                        if(mfc_data->type == MfClassicType4k) block_count = 256;
                        for(size_t i = 0; i < block_count; i++) {
                            FURI_LOG_D(TAG, "Writing block %d", i);
                            if(!magic_gen4_write_blk(password, i, mfc_data->block[i].value)) {
                                FURI_LOG_E(TAG, "Failed to write %d block", i);
                                nfc_magic_worker->callback(
                                    NfcMagicWorkerEventFail, nfc_magic_worker->context);
                                done = true;
                                break;
                            }
                        }
                    } else if(dev_protocol == NfcDeviceProtocolMifareUl) {
                        MfUltralightData* mf_ul_data = &dev_data->mf_ul_data;
                        for(size_t i = 0; (i * 4) < mf_ul_data->data_read; i++) {
                            size_t data_offset = i * 4;
                            FURI_LOG_D(
                                TAG,
                                "Writing page %zu (%zu/%u)",
                                i,
                                data_offset,
                                mf_ul_data->data_read);
                            uint8_t* block = mf_ul_data->data + data_offset;
                            if(!magic_gen4_write_blk(password, i, block)) {
                                FURI_LOG_E(TAG, "Failed to write %zu page", i);
                                nfc_magic_worker->callback(
                                    NfcMagicWorkerEventFail, nfc_magic_worker->context);
                                done = true;
                                break;
                            }
                        }

                        uint8_t buffer[16] = {0};

                        for(size_t i = 0; i < 8; i++) {
                            memcpy(buffer, &mf_ul_data->signature[i * 4], 4); //-V1086
                            if(!magic_gen4_write_blk(password, 0xF2 + i, buffer)) {
                                FURI_LOG_E(TAG, "Failed to write signature block %d", i);
                                nfc_magic_worker->callback(
                                    NfcMagicWorkerEventFail, nfc_magic_worker->context);
                                done = true;
                                break;
                            }
                        }

                        buffer[0] = mf_ul_data->version.header;
                        buffer[1] = mf_ul_data->version.vendor_id;
                        buffer[2] = mf_ul_data->version.prod_type;
                        buffer[3] = mf_ul_data->version.prod_subtype;
                        if(!magic_gen4_write_blk(password, 0xFA, buffer)) {
                            FURI_LOG_E(TAG, "Failed to write version block 0");
                            nfc_magic_worker->callback(
                                NfcMagicWorkerEventFail, nfc_magic_worker->context);
                            done = true;
                            break;
                        }

                        buffer[0] = mf_ul_data->version.prod_ver_major;
                        buffer[1] = mf_ul_data->version.prod_ver_minor;
                        buffer[2] = mf_ul_data->version.storage_size;
                        buffer[3] = mf_ul_data->version.protocol_type;
                        if(!magic_gen4_write_blk(password, 0xFB, buffer)) {
                            FURI_LOG_E(TAG, "Failed to write version block 1");
                            nfc_magic_worker->callback(
                                NfcMagicWorkerEventFail, nfc_magic_worker->context);
                            done = true;
                            break;
                        }
                    }

                    nfc_magic_worker->callback(
                        NfcMagicWorkerEventSuccess, nfc_magic_worker->context);
                    done = true;
                    break;
                }
            }
        } while(false);

        if(done) break;

        if(card_found_notified) {
            nfc_magic_worker->callback(
                NfcMagicWorkerEventNoCardDetected, nfc_magic_worker->context);
            card_found_notified = false;
        }

        furi_delay_ms(300);
    }
    magic_deactivate();
}

void nfc_magic_worker_check(NfcMagicWorker* nfc_magic_worker) {
    FuriHalNfcDevData nfc_data = {};
    NfcMagicDevice* magic_dev = nfc_magic_worker->magic_dev;
    bool card_found_notified = false;
    uint8_t gen4_config[MAGIC_GEN4_CONFIG_LEN];

    while(nfc_magic_worker->state == NfcMagicWorkerStateCheck) {
        magic_activate();
        if(magic_gen1_wupa()) {
            magic_dev->type = MagicTypeClassicGen1;
            if(!card_found_notified) {
                nfc_magic_worker->callback(
                    NfcMagicWorkerEventCardDetected, nfc_magic_worker->context);
                card_found_notified = true;
            }

            if(furi_hal_nfc_detect(&nfc_data, 200)) {
                magic_dev->cuid = nfc_data.cuid;
                magic_dev->uid_len = nfc_data.uid_len;
            } else {
                // wrong BCC
                magic_dev->uid_len = 4;
            }
            nfc_magic_worker->callback(NfcMagicWorkerEventSuccess, nfc_magic_worker->context);
            break;
        } else {
            magic_deactivate();
            magic_activate();
            if(furi_hal_nfc_detect(&nfc_data, 200)) {
                magic_dev->cuid = nfc_data.cuid;
                magic_dev->uid_len = nfc_data.uid_len;
                if(magic_gen4_get_cfg(magic_dev->password, gen4_config)) {
                    magic_dev->type = MagicTypeGen4;
                    if(!card_found_notified) {
                        nfc_magic_worker->callback(
                            NfcMagicWorkerEventCardDetected, nfc_magic_worker->context);
                        card_found_notified = true;
                    }

                    nfc_magic_worker->callback(
                        NfcMagicWorkerEventSuccess, nfc_magic_worker->context);
                } else {
                    nfc_magic_worker->callback(
                        NfcMagicWorkerEventWrongCard, nfc_magic_worker->context);
                    card_found_notified = true;
                }
                break;
            } else {
                if(card_found_notified) {
                    nfc_magic_worker->callback(
                        NfcMagicWorkerEventNoCardDetected, nfc_magic_worker->context);
                    card_found_notified = false;
                }
            }
        }

        magic_deactivate();
        furi_delay_ms(300);
    }

    magic_deactivate();
}

void nfc_magic_worker_rekey(NfcMagicWorker* nfc_magic_worker) {
    NfcMagicDevice* magic_dev = nfc_magic_worker->magic_dev;
    bool card_found_notified = false;

    if(magic_dev->type != MagicTypeGen4) {
        nfc_magic_worker->callback(NfcMagicWorkerEventCardDetected, nfc_magic_worker->context);
        return;
    }

    while(nfc_magic_worker->state == NfcMagicWorkerStateRekey) {
        magic_activate();
        uint32_t cuid;
        furi_hal_nfc_activate_nfca(200, &cuid);
        if(cuid != magic_dev->cuid) {
            if(card_found_notified) {
                nfc_magic_worker->callback(
                    NfcMagicWorkerEventNoCardDetected, nfc_magic_worker->context);
                card_found_notified = false;
            }
            continue;
        }

        nfc_magic_worker->callback(NfcMagicWorkerEventCardDetected, nfc_magic_worker->context);
        card_found_notified = true;

        if(magic_gen4_set_pwd(magic_dev->password, nfc_magic_worker->new_password)) {
            magic_dev->password = nfc_magic_worker->new_password;
            nfc_magic_worker->callback(NfcMagicWorkerEventSuccess, nfc_magic_worker->context);
            break;
        }

        if(card_found_notified) { //-V547
            nfc_magic_worker->callback(
                NfcMagicWorkerEventNoCardDetected, nfc_magic_worker->context);
            card_found_notified = false;
        }
        furi_delay_ms(300);
    }
    magic_deactivate();
}

void nfc_magic_worker_wipe(NfcMagicWorker* nfc_magic_worker) {
    NfcMagicDevice* magic_dev = nfc_magic_worker->magic_dev;
    bool card_found_notified = false;
    bool card_wiped = false;

    MfClassicBlock block;
    memset(&block, 0, sizeof(MfClassicBlock));
    MfClassicBlock empty_block;
    memset(&empty_block, 0, sizeof(MfClassicBlock));
    MfClassicBlock trailer_block;
    memset(&trailer_block, 0xff, sizeof(MfClassicBlock));

    block.value[0] = 0x01;
    block.value[1] = 0x02;
    block.value[2] = 0x03;
    block.value[3] = 0x04;
    block.value[4] = 0x04;
    block.value[5] = 0x08;
    block.value[6] = 0x04;

    trailer_block.value[7] = 0x07;
    trailer_block.value[8] = 0x80;
    trailer_block.value[9] = 0x69;

    while(nfc_magic_worker->state == NfcMagicWorkerStateWipe) {
        do {
            magic_deactivate();
            furi_delay_ms(300);
            if(!magic_activate()) break;
            if(magic_dev->type == MagicTypeClassicGen1) {
                if(!magic_gen1_wupa()) break;
                if(!card_found_notified) {
                    nfc_magic_worker->callback(
                        NfcMagicWorkerEventCardDetected, nfc_magic_worker->context);
                    card_found_notified = true;
                }

                if(!magic_gen1_data_access_cmd()) break;
                if(!magic_gen1_write_blk(0, &block)) break;

                for(size_t i = 1; i < 64; i++) {
                    FURI_LOG_D(TAG, "Wiping block %d", i);
                    bool success = false;
                    if((i | 0x03) == i) {
                        success = magic_gen1_write_blk(i, &trailer_block);
                    } else {
                        success = magic_gen1_write_blk(i, &empty_block);
                    }

                    if(!success) {
                        FURI_LOG_E(TAG, "Failed to write %d block", i);
                        nfc_magic_worker->callback(
                            NfcMagicWorkerEventFail, nfc_magic_worker->context);
                        break;
                    }
                }

                card_wiped = true;
                nfc_magic_worker->callback(NfcMagicWorkerEventSuccess, nfc_magic_worker->context);
            } else if(magic_dev->type == MagicTypeGen4) {
                uint32_t cuid;
                if(!furi_hal_nfc_activate_nfca(200, &cuid)) break;
                if(cuid != magic_dev->cuid) break;
                if(!card_found_notified) {
                    nfc_magic_worker->callback(
                        NfcMagicWorkerEventCardDetected, nfc_magic_worker->context);
                    card_found_notified = true;
                }

                if(!magic_gen4_wipe(magic_dev->password)) break;

                card_wiped = true;
                nfc_magic_worker->callback(NfcMagicWorkerEventSuccess, nfc_magic_worker->context);
            }
        } while(false);

        if(card_wiped) break;

        if(card_found_notified) {
            nfc_magic_worker->callback(
                NfcMagicWorkerEventNoCardDetected, nfc_magic_worker->context);
            card_found_notified = false;
        }
    }
    magic_deactivate();
}

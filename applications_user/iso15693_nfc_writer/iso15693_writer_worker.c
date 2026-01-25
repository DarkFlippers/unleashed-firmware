#include "iso15693_writer_worker.h"

struct Iso15693WriterWorker {
    Nfc* nfc;
    NfcPoller* poller;
    Iso15693WriterWorkerCallback callback;
    void* context;
    uint8_t write_block;
    uint8_t single_data[32];
    uint8_t* full_data;
    uint8_t* lock_status;
    uint8_t afi_value;
    uint8_t dsfid_value;
    uint8_t progress;
    uint16_t block_count;
    uint8_t block_size;
    Iso15693WriterWorkerMode mode;
    bool is_running;
};

static bool iso15693_send_read_with_status(Iso15693_3Poller* poller, uint8_t block, uint8_t* dest, uint8_t* status, uint8_t block_size) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x42);
    bit_buffer_append_byte(tx, 0x20);
    bit_buffer_append_byte(tx, block);
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = false;

    if(err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) >= (size_t)(2 + block_size) && (bit_buffer_get_byte(rx, 0) & 0x01) == 0) {
        *status = bit_buffer_get_byte(rx, 1);
        for(int i=0; i<block_size; i++) dest[i] = bit_buffer_get_byte(rx, i+2);
        ok = true;
    }
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_get_system_info(Iso15693_3Poller* poller, uint16_t* block_count, uint8_t* block_size) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x02); 
    bit_buffer_append_byte(tx, 0x2B); 
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = false;
    
    if(err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) >= 10 && (bit_buffer_get_byte(rx, 0) & 0x01) == 0) {
        uint8_t info_flags = bit_buffer_get_byte(rx, 1);
        size_t offset = 10; 
        
        if(info_flags & 0x01) offset += 1;
        if(info_flags & 0x02) offset += 1;
        
        if(info_flags & 0x04) {
            if(bit_buffer_get_size_bytes(rx) >= offset + 2) {
                *block_count = bit_buffer_get_byte(rx, offset) + 1;
                *block_size = (bit_buffer_get_byte(rx, offset + 1) & 0x1F) + 1;
                ok = true;
            }
        }
    }
    
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_write(Iso15693_3Poller* poller, uint8_t block, const uint8_t* data, uint8_t block_size) {
    if(block_size > 32) return false;
    
    BitBuffer* tx = bit_buffer_alloc(40);
    bit_buffer_append_byte(tx, 0x02);
    bit_buffer_append_byte(tx, 0x21);
    bit_buffer_append_byte(tx, block);
    
    for(uint8_t i = 0; i < block_size; i++) {
        bit_buffer_append_byte(tx, data[i]);
    }
    
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = (err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) > 0 && (bit_buffer_get_byte(rx, 0) & 0x01) == 0);
    
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_read(Iso15693_3Poller* poller, uint8_t block, uint8_t* dest, uint8_t block_size) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x02);
    bit_buffer_append_byte(tx, 0x20);
    bit_buffer_append_byte(tx, block);
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = false;
    if(err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) >= (size_t)(1 + block_size) && (bit_buffer_get_byte(rx, 0) & 0x01) == 0) {
        for(int i=0; i<block_size; i++) dest[i] = bit_buffer_get_byte(rx, i+1);
        ok = true;
    }
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_lock(Iso15693_3Poller* poller, uint8_t block) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x02);
    bit_buffer_append_byte(tx, 0x22);
    bit_buffer_append_byte(tx, block);
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = (err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) > 0 && (bit_buffer_get_byte(rx, 0) & 0x01) == 0);
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_write_afi(Iso15693_3Poller* poller, uint8_t afi_value) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x02);
    bit_buffer_append_byte(tx, 0x27);
    bit_buffer_append_byte(tx, afi_value);
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = (err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) > 0 && (bit_buffer_get_byte(rx, 0) & 0x01) == 0);
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_lock_afi(Iso15693_3Poller* poller) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x02);
    bit_buffer_append_byte(tx, 0x28);
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = (err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) > 0 && (bit_buffer_get_byte(rx, 0) & 0x01) == 0);
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_write_dsfid(Iso15693_3Poller* poller, uint8_t dsfid_value) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x02);
    bit_buffer_append_byte(tx, 0x29);
    bit_buffer_append_byte(tx, dsfid_value);
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = (err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) > 0 && (bit_buffer_get_byte(rx, 0) & 0x01) == 0);
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static bool iso15693_send_lock_dsfid(Iso15693_3Poller* poller) {
    BitBuffer* tx = bit_buffer_alloc(32);
    bit_buffer_append_byte(tx, 0x02);
    bit_buffer_append_byte(tx, 0x2A);
    BitBuffer* rx = bit_buffer_alloc(40);
    Iso15693_3Error err = iso15693_3_poller_send_frame(poller, tx, rx, 300000);
    bool ok = (err == Iso15693_3ErrorNone && bit_buffer_get_size_bytes(rx) > 0 && (bit_buffer_get_byte(rx, 0) & 0x01) == 0);
    bit_buffer_free(tx); bit_buffer_free(rx);
    return ok;
}

static NfcCommand iso15693_writer_poller_callback(NfcGenericEvent event, void* context) {
    Iso15693WriterWorker* instance = context;
    if(event.protocol == NfcProtocolIso15693_3) {
        Iso15693_3PollerEvent* iso15693_event = event.event_data;
        if(iso15693_event->type == Iso15693_3PollerEventTypeReady) {
            Iso15693_3Poller* iso15693_poller = (Iso15693_3Poller*)event.instance;
            
            
            uint16_t detected_block_count = 0;
            uint8_t detected_block_size = 0;
            if(iso15693_send_get_system_info(iso15693_poller, &detected_block_count, &detected_block_size)) {
                instance->block_count = detected_block_count;
                instance->block_size = detected_block_size;
            } else {
                
                instance->block_count = 28; 
                instance->block_size = 4;
            }
            
            if(instance->callback) instance->callback(Iso15693WriterWorkerEventCardDetected, instance->context);

            bool success = true;
            Iso15693WriterWorkerEvent result_event = Iso15693WriterWorkerEventWriteFail;

            if(instance->mode == Iso15693WriterWorkerModeFormat) {
                static const uint8_t ff_data[32] = {
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
                };
                for(uint16_t b = 0; b < instance->block_count; b++) {
                    instance->progress = (b > 255) ? 255 : (uint8_t)b;
                    if(b % 4 == 0 || b == instance->block_count - 1) {
                        if(instance->callback) instance->callback(Iso15693WriterWorkerEventProgress, instance->context);
                    }
                    iso15693_send_write(iso15693_poller, (uint8_t)b, ff_data, instance->block_size);
                    furi_delay_ms(5);
                }
                result_event = Iso15693WriterWorkerEventWriteSuccess;
            } else if(instance->mode == Iso15693WriterWorkerModeWriteFull) {
                for(uint16_t b = 0; b < instance->block_count; b++) {
                    instance->progress = (b > 255) ? 255 : (uint8_t)b;
                    if(b % 4 == 0 || b == instance->block_count - 1) {
                         if(instance->callback) instance->callback(Iso15693WriterWorkerEventProgress, instance->context);
                    }
                    iso15693_send_write(iso15693_poller, (uint8_t)b, &instance->full_data[b * instance->block_size], instance->block_size);
                    furi_delay_ms(5);
                }
                result_event = Iso15693WriterWorkerEventWriteSuccess;
            } else if(instance->mode == Iso15693WriterWorkerModeReadAll) {
                for(uint16_t b = 0; b < instance->block_count; b++) {
                    instance->progress = (b > 255) ? 255 : (uint8_t)b;
                    if(b % 4 == 0 || b == instance->block_count - 1) {
                        if(instance->callback) instance->callback(Iso15693WriterWorkerEventProgress, instance->context);
                    }
                    
                    uint8_t status = 0;
                    if(!iso15693_send_read_with_status(iso15693_poller, (uint8_t)b, &instance->full_data[b * instance->block_size], &status, instance->block_size)) {
                        if(!iso15693_send_read(iso15693_poller, (uint8_t)b, &instance->full_data[b * instance->block_size], instance->block_size)) {
                            memset(&instance->full_data[b * instance->block_size], 0xFF, instance->block_size);
                            status = 0xFF;
                            success = false;
                        } else {
                            status = 0;
                        }
                    }
                    if(instance->lock_status) instance->lock_status[b] = status;
                    
                    furi_delay_ms(5);
                }
                result_event = success ? Iso15693WriterWorkerEventReadSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeWrite) {
                success = iso15693_send_write(iso15693_poller, instance->write_block, instance->single_data, instance->block_size);
                result_event = success ? Iso15693WriterWorkerEventWriteSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeRead) {
                uint8_t status = 0;
                success = iso15693_send_read_with_status(iso15693_poller, instance->write_block, &instance->full_data[instance->write_block * instance->block_size], &status, instance->block_size);
                if(!success) {
                    success = iso15693_send_read(iso15693_poller, instance->write_block, &instance->full_data[instance->write_block * instance->block_size], instance->block_size);
                    status = 0;
                }
                if(instance->lock_status) instance->lock_status[instance->write_block] = status;
                result_event = success ? Iso15693WriterWorkerEventReadSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeLock) {
                success = iso15693_send_lock(iso15693_poller, instance->write_block);
                result_event = success ? Iso15693WriterWorkerEventWriteSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeLockAll) {
                for(uint16_t b = 0; b < instance->block_count; b++) {
                    instance->progress = (b > 255) ? 255 : (uint8_t)b;
                    if(b % 8 == 0 || b == instance->block_count - 1) {
                        if(instance->callback) instance->callback(Iso15693WriterWorkerEventProgress, instance->context);
                    }
                    if(!iso15693_send_lock(iso15693_poller, (uint8_t)b)) { success = false; break; }
                    furi_delay_ms(10);
                }
                result_event = success ? Iso15693WriterWorkerEventWriteSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeWriteAFI) {
                success = iso15693_send_write_afi(iso15693_poller, instance->afi_value);
                result_event = success ? Iso15693WriterWorkerEventWriteSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeLockAFI) {
                success = iso15693_send_lock_afi(iso15693_poller);
                result_event = success ? Iso15693WriterWorkerEventWriteSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeWriteDSFID) {
                success = iso15693_send_write_dsfid(iso15693_poller, instance->dsfid_value);
                result_event = success ? Iso15693WriterWorkerEventWriteSuccess : Iso15693WriterWorkerEventWriteFail;
            } else if(instance->mode == Iso15693WriterWorkerModeLockDSFID) {
                success = iso15693_send_lock_dsfid(iso15693_poller);
                result_event = success ? Iso15693WriterWorkerEventWriteSuccess : Iso15693WriterWorkerEventWriteFail;
            }

            if(instance->callback) instance->callback(success ? result_event : Iso15693WriterWorkerEventWriteFail, instance->context);
            return NfcCommandStop;
        }
    }
    return NfcCommandContinue;
}

Iso15693WriterWorker* iso15693_writer_worker_alloc() {
    return malloc(sizeof(Iso15693WriterWorker));
}

void iso15693_writer_worker_free(Iso15693WriterWorker* instance) {
    iso15693_writer_worker_stop(instance);
    free(instance);
}

void iso15693_writer_worker_start(Iso15693WriterWorker* instance, Iso15693WriterWorkerMode mode, uint8_t block, uint8_t* full_data, uint8_t* lock_status, const uint8_t* single_data, uint8_t afi, uint8_t dsfid, Iso15693WriterWorkerCallback cb, void* ctx) {
    memset(instance, 0, sizeof(Iso15693WriterWorker));
    instance->mode = mode;
    instance->write_block = block;
    instance->full_data = full_data;
    instance->lock_status = lock_status;
    if(single_data) memcpy(instance->single_data, single_data, 32); 
    
    
    instance->block_count = 28;
    instance->block_size = 4;
    
    instance->afi_value = afi;
    instance->dsfid_value = dsfid;
    instance->callback = cb;
    instance->context = ctx;
    
    instance->nfc = nfc_alloc();
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolIso15693_3);
    instance->is_running = true;
    nfc_poller_start(instance->poller, iso15693_writer_poller_callback, instance);
}

void iso15693_writer_worker_stop(Iso15693WriterWorker* instance) {
    if(!instance || !instance->is_running) return;
    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);
    nfc_free(instance->nfc);
    instance->is_running = false;
}

uint8_t iso15693_writer_worker_get_progress(Iso15693WriterWorker* instance) {
    return instance->progress;
}

uint16_t iso15693_writer_worker_get_block_count(Iso15693WriterWorker* instance) {
    return instance->block_count;
}

uint8_t iso15693_writer_worker_get_block_size(Iso15693WriterWorker* instance) {
    return instance->block_size;
}
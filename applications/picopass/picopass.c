#include "picopass.h"
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <st25r3916.h>
#include <rfal_analogConfig.h>
#include <rfal_rf.h>
#include <rfal_nfc.h>

#include <storage/storage.h>
#include <lib/toolbox/path.h>

#define TAG "PicoPass"

#define PICOPASS_APP_ICLASS_KEY_PATH "/any/picopass/iclass_key.bin"
#define PICOPASS_APP_ICLASS_DECRYPT_KEY_PATH "/any/picopass/iclass_decryptionkey.bin"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    bool valid;
    uint8_t bitLength;
    uint8_t FacilityCode;
    uint16_t CardNumber;
} WiegandRecord;

typedef struct {
    bool biometrics;
    uint8_t encryption;
    uint8_t credential[8];
    uint8_t pin0[8];
    uint8_t pin1[8];
    WiegandRecord record;
} PACS;

enum State { INIT, KEYS_MISSING, READY, RESULT };
typedef struct {
    enum State state;
    PACS pacs;
} PluginState;

uint8_t iclass_key[8] = {0}; // NB: not the permuted version
uint8_t iclass_decryptionkey[16] = {0};
ApplicationArea AA1;

static bool picopass_load_keys() {
    Storage* storage = furi_record_open("storage");
    File* file = storage_file_alloc(storage);

    if(!storage_file_open(file, PICOPASS_APP_ICLASS_KEY_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Unable to open iClass key");
        storage_file_free(file);
        furi_record_close("storage");
        return false;
    };
    storage_file_read(file, iclass_key, sizeof(iclass_key));
    storage_file_close(file);
    FURI_LOG_D(TAG, "iClass key loaded");

    if(!storage_file_open(
           file, PICOPASS_APP_ICLASS_DECRYPT_KEY_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Unable to open iClass decryption key");
        storage_file_free(file);
        furi_record_close("storage");
        return false;
    };
    storage_file_read(file, iclass_decryptionkey, sizeof(iclass_decryptionkey));
    storage_file_close(file);
    FURI_LOG_D(TAG, "iClass decryption key loaded");

    storage_file_free(file);
    furi_record_close("storage");
    return true;
}

static void render_callback(Canvas* const canvas, void* ctx) {
    const PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }
    // border around the edge of the screen
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    canvas_set_font(canvas, FontPrimary);

    if(plugin_state->state == INIT) {
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, "Loading...");
    } else if(plugin_state->state == KEYS_MISSING) {
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, "Keys missing");
    } else if(plugin_state->state == READY) {
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignTop, "Push center to scan");
    } else if(plugin_state->state == RESULT) {
        char raw_credential[25] = {0};
        sprintf(
            raw_credential,
            "%02x %02x %02x %02x %02x %02x %02x %02x",
            plugin_state->pacs.credential[0],
            plugin_state->pacs.credential[1],
            plugin_state->pacs.credential[2],
            plugin_state->pacs.credential[3],
            plugin_state->pacs.credential[4],
            plugin_state->pacs.credential[5],
            plugin_state->pacs.credential[6],
            plugin_state->pacs.credential[7]);
        canvas_draw_str_aligned(canvas, 64, 34, AlignCenter, AlignTop, raw_credential);

        if(plugin_state->pacs.record.valid) {
            char parsed[20] = {0};
            sprintf(
                parsed,
                "FC: %03u CN: %05u",
                plugin_state->pacs.record.FacilityCode,
                plugin_state->pacs.record.CardNumber);
            canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, parsed);
        }
    }

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, osMessageQueueId_t event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

static void picopass_state_init(PluginState* const plugin_state) {
    plugin_state->state = INIT;
    if(picopass_load_keys()) {
        plugin_state->state = READY;
    } else {
        plugin_state->state = KEYS_MISSING;
    }
}

ReturnCode decrypt(uint8_t* enc_data, uint8_t* dec_data) {
    uint8_t key[32] = {0};
    memcpy(key, iclass_decryptionkey, sizeof(iclass_decryptionkey));
    mbedtls_des3_context ctx;
    mbedtls_des3_init(&ctx);
    mbedtls_des3_set2key_dec(&ctx, key);
    mbedtls_des3_crypt_ecb(&ctx, enc_data, dec_data);
    mbedtls_des3_free(&ctx);
    return ERR_NONE;
}

ReturnCode parseWiegand(uint8_t* data, WiegandRecord* record) {
    uint32_t* halves = (uint32_t*)data;
    if(halves[0] == 0) {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[1]));
        record->bitLength = 31 - leading0s;
    } else {
        uint8_t leading0s = __builtin_clz(REVERSE_BYTES_U32(halves[0]));
        record->bitLength = 63 - leading0s;
    }
    FURI_LOG_D(TAG, "bitLength: %d", record->bitLength);

    if(record->bitLength == 26) {
        uint8_t* v4 = data + 4;
        v4[0] = 0;

        uint32_t bot = v4[3] | (v4[2] << 8) | (v4[1] << 16) | (v4[0] << 24);

        record->CardNumber = (bot >> 1) & 0xFFFF;
        record->FacilityCode = (bot >> 17) & 0xFF;
        record->valid = true;
    } else {
        record->CardNumber = 0;
        record->FacilityCode = 0;
        record->valid = false;
    }
    return ERR_NONE;
}

ReturnCode disable_field(ReturnCode rc) {
    st25r3916TxRxOff();
    rfalLowPowerModeStart();
    return rc;
}

ReturnCode picopass_read_card(ApplicationArea* AA1) {
    rfalPicoPassIdentifyRes idRes;
    rfalPicoPassSelectRes selRes;
    rfalPicoPassReadCheckRes rcRes;
    rfalPicoPassCheckRes chkRes;

    ReturnCode err;

    uint8_t div_key[8] = {0};
    uint8_t mac[4] = {0};
    uint8_t ccnr[12] = {0};

    st25r3916TxRxOn();
    rfalLowPowerModeStop();
    rfalWorker();
    err = rfalPicoPassPollerInitialize();
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerInitialize error %d\n", err);
        return disable_field(err);
    }

    err = rfalFieldOnAndStartGT();
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalFieldOnAndStartGT error %d\n", err);
        return disable_field(err);
    }

    err = rfalPicoPassPollerCheckPresence();
    if(err != ERR_RF_COLLISION) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerCheckPresence error %d\n", err);
        return disable_field(err);
    }

    err = rfalPicoPassPollerIdentify(&idRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerIdentify error %d\n", err);
        return disable_field(err);
    }

    err = rfalPicoPassPollerSelect(idRes.CSN, &selRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerSelect error %d\n", err);
        return disable_field(err);
    }

    err = rfalPicoPassPollerReadCheck(&rcRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerReadCheck error %d", err);
        return disable_field(err);
    }
    memcpy(ccnr, rcRes.CCNR, sizeof(rcRes.CCNR)); // last 4 bytes left 0

    diversifyKey(selRes.CSN, iclass_key, div_key);
    opt_doReaderMAC(ccnr, div_key, mac);

    err = rfalPicoPassPollerCheck(mac, &chkRes);
    if(err != ERR_NONE) {
        FURI_LOG_E(TAG, "rfalPicoPassPollerCheck error %d", err);
        return disable_field(err);
    }

    for(size_t i = 0; i < 4; i++) {
        FURI_LOG_D(TAG, "rfalPicoPassPollerReadBlock block %d", i + 6);
        err = rfalPicoPassPollerReadBlock(i + 6, &(AA1->block[i]));
        if(err != ERR_NONE) {
            FURI_LOG_E(TAG, "rfalPicoPassPollerReadBlock error %d", err);
            return disable_field(err);
        }
    }
    return disable_field(ERR_NONE);
}

int32_t picopass_app(void* p) {
    UNUSED(p);
    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(PluginEvent), NULL);

    PluginState* plugin_state = malloc(sizeof(PluginState));
    picopass_state_init(plugin_state);
    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("Hello_world", "cannot create mutex\r\n");
        free(plugin_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    ReturnCode err;
    for(bool processing = true; processing;) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, 100);
        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == osOK) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        FURI_LOG_D(TAG, "Input Up");
                        break;
                    case InputKeyDown:
                        FURI_LOG_D(TAG, "Input Down");
                        break;
                    case InputKeyRight:
                        FURI_LOG_D(TAG, "Input Right");
                        break;
                    case InputKeyLeft:
                        FURI_LOG_D(TAG, "Input Left");
                        break;
                    case InputKeyOk:
                        FURI_LOG_D(TAG, "Input OK");
                        err = picopass_read_card(&AA1);
                        if(err != ERR_NONE) {
                            FURI_LOG_E(TAG, "picopass_read_card error %d", err);
                            plugin_state->state = READY;
                            break;
                        }
                        FURI_LOG_D(TAG, "read OK");

                        plugin_state->pacs.biometrics = AA1.block[0].data[4];
                        plugin_state->pacs.encryption = AA1.block[0].data[7];
                        if(plugin_state->pacs.encryption == 0x17) {
                            FURI_LOG_D(TAG, "3DES Encrypted");
                            err = decrypt(AA1.block[1].data, plugin_state->pacs.credential);
                            if(err != ERR_NONE) {
                                FURI_LOG_E(TAG, "decrypt error %d", err);
                                break;
                            }
                            FURI_LOG_D(TAG, "Decrypted 7");

                            err = decrypt(AA1.block[2].data, plugin_state->pacs.pin0);
                            if(err != ERR_NONE) {
                                FURI_LOG_E(TAG, "decrypt error %d", err);
                                break;
                            }
                            FURI_LOG_D(TAG, "Decrypted 8");

                            err = decrypt(AA1.block[3].data, plugin_state->pacs.pin1);
                            if(err != ERR_NONE) {
                                FURI_LOG_E(TAG, "decrypt error %d", err);
                                break;
                            }
                            FURI_LOG_D(TAG, "Decrypted 9");
                        } else if(plugin_state->pacs.encryption == 0x14) {
                            FURI_LOG_D(TAG, "No Encryption");
                            memcpy(
                                plugin_state->pacs.credential,
                                AA1.block[1].data,
                                RFAL_PICOPASS_MAX_BLOCK_LEN);
                            memcpy(
                                plugin_state->pacs.pin0,
                                AA1.block[2].data,
                                RFAL_PICOPASS_MAX_BLOCK_LEN);
                            memcpy(
                                plugin_state->pacs.pin1,
                                AA1.block[3].data,
                                RFAL_PICOPASS_MAX_BLOCK_LEN);
                        } else if(plugin_state->pacs.encryption == 0x15) {
                            FURI_LOG_D(TAG, "DES Encrypted");
                        } else {
                            FURI_LOG_D(TAG, "Unknown encryption");
                            break;
                        }

                        FURI_LOG_D(
                            TAG,
                            "credential %02x%02x%02x%02x%02x%02x%02x%02x",
                            plugin_state->pacs.credential[0],
                            plugin_state->pacs.credential[1],
                            plugin_state->pacs.credential[2],
                            plugin_state->pacs.credential[3],
                            plugin_state->pacs.credential[4],
                            plugin_state->pacs.credential[5],
                            plugin_state->pacs.credential[6],
                            plugin_state->pacs.credential[7]);
                        FURI_LOG_D(
                            TAG,
                            "pin0 %02x%02x%02x%02x%02x%02x%02x%02x",
                            plugin_state->pacs.pin0[0],
                            plugin_state->pacs.pin0[1],
                            plugin_state->pacs.pin0[2],
                            plugin_state->pacs.pin0[3],
                            plugin_state->pacs.pin0[4],
                            plugin_state->pacs.pin0[5],
                            plugin_state->pacs.pin0[6],
                            plugin_state->pacs.pin0[7]);
                        FURI_LOG_D(
                            TAG,
                            "pin1 %02x%02x%02x%02x%02x%02x%02x%02x",
                            plugin_state->pacs.pin1[0],
                            plugin_state->pacs.pin1[1],
                            plugin_state->pacs.pin1[2],
                            plugin_state->pacs.pin1[3],
                            plugin_state->pacs.pin1[4],
                            plugin_state->pacs.pin1[5],
                            plugin_state->pacs.pin1[6],
                            plugin_state->pacs.pin1[7]);

                        err = parseWiegand(
                            plugin_state->pacs.credential, &plugin_state->pacs.record);
                        if(err != ERR_NONE) {
                            FURI_LOG_E(TAG, "parse error %d", err);
                            break;
                        }
                        if(plugin_state->pacs.record.valid) {
                            FURI_LOG_D(
                                TAG,
                                "FC: %03d CN: %05d",
                                plugin_state->pacs.record.FacilityCode,
                                plugin_state->pacs.record.CardNumber);
                        }
                        plugin_state->state = RESULT;

                        break;
                    case InputKeyBack:
                        FURI_LOG_D(TAG, "Input Back");
                        processing = false;
                        break;
                    }
                }
            }
        } else {
            // FURI_LOG_D(TAG, "osMessageQueue: event timeout");
            // event timeout
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);

    return 0;
}

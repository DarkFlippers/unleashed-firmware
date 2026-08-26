#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>

#include <nfc/nfc_device.h>
#include <nfc/helpers/nfc_data_generator.h>
#include <nfc/nfc_poller.h>
#include <nfc/nfc_listener.h>
#include <nfc/protocols/iso14443_3a/iso14443_3a.h>
#include <nfc/protocols/iso14443_3a/iso14443_3a_poller.h>
#include <nfc/protocols/iso14443_3a/iso14443_3a_poller_sync.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight.h>
#include <nfc/protocols/mf_ultralight/mf_ultralight_poller_sync.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include <nfc/protocols/felica/felica.h>
#include <nfc/protocols/felica/felica_poller_sync.h>
#include <nfc/protocols/mf_classic/mf_classic_poller.h>
#include <nfc/protocols/iso15693_3/iso15693_3_poller.h>
#include <nfc/protocols/slix/slix.h>
#include <nfc/protocols/slix/slix_i.h>
#include <nfc/protocols/slix/slix_poller.h>
#include <nfc/protocols/slix/slix_poller_i.h>
#include <nfc/protocols/mf_plus/mf_plus_crypto.h>

#include <nfc/nfc_poller.h>

#include <toolbox/keys_dict.h>
#include <nfc/nfc.h>

#include "../test.h" // IWYU pragma: keep

#define TAG "NfcTest"

#define NFC_TEST_NFC_DEV_PATH                  EXT_PATH("unit_tests/nfc/nfc_device_test.nfc")
#define NFC_APP_MF_CLASSIC_DICT_UNIT_TEST_PATH EXT_PATH("unit_tests/mf_dict.nfc")

#define NFC_TEST_FLAG_WORKER_DONE (1)

typedef enum {
    NfcTestMfClassicSendFrameTestStateAuth,
    NfcTestMfClassicSendFrameTestStateReadBlock,

    NfcTestMfClassicSendFrameTestStateFail,
    NfcTestMfClassicSendFrameTestStateSuccess,
} NfcTestMfClassicSendFrameTestState;

typedef struct {
    NfcTestMfClassicSendFrameTestState state;
    BitBuffer* tx_buf;
    BitBuffer* rx_buf;
    FuriThreadId thread_id;
} NfcTestMfClassicSendFrameTest;

typedef enum {
    NfcTestSlixPollerSetPasswordStateGetRandomNumber,
    NfcTestSlixPollerSetPasswordStateSetPassword,
} NfcTestSlixPollerSetPasswordState;

typedef struct {
    FuriThreadId thread_id;
    NfcTestSlixPollerSetPasswordState state;
    SlixRandomNumber random_number;
    SlixPassword password;
    SlixError error;
} NfcTestSlixPollerSetPasswordContext;

typedef struct {
    Storage* storage;
} NfcTest;

static NfcTest* nfc_test = NULL;

static void nfc_test_alloc(void) {
    nfc_test = malloc(sizeof(NfcTest));
    nfc_test->storage = furi_record_open(RECORD_STORAGE);
}

static void nfc_test_free(void) {
    furi_check(nfc_test);

    furi_record_close(RECORD_STORAGE);
    free(nfc_test);
    nfc_test = NULL;
}

static void nfc_test_save_and_load(NfcDevice* nfc_device_ref) {
    NfcDevice* nfc_device_dut = nfc_device_alloc();

    mu_assert(
        nfc_device_save(nfc_device_ref, NFC_TEST_NFC_DEV_PATH), "nfc_device_save() failed\r\n");

    mu_assert(
        nfc_device_load(nfc_device_dut, NFC_TEST_NFC_DEV_PATH), "nfc_device_load() failed\r\n");

    mu_assert(
        nfc_device_is_equal(nfc_device_ref, nfc_device_dut),
        "nfc_device_data_dut != nfc_device_data_ref\r\n");

    mu_assert(
        storage_simply_remove(nfc_test->storage, NFC_TEST_NFC_DEV_PATH),
        "storage_simply_remove() failed\r\n");

    nfc_device_free(nfc_device_dut);
}

static void iso14443_3a_file_test(uint8_t uid_len) {
    NfcDevice* nfc_device = nfc_device_alloc();

    Iso14443_3aData* data = iso14443_3a_alloc();
    data->uid_len = uid_len;
    furi_hal_random_fill_buf(data->uid, uid_len);
    furi_hal_random_fill_buf(data->atqa, sizeof(data->atqa));
    furi_hal_random_fill_buf(&data->sak, 1);

    nfc_device_set_data(nfc_device, NfcProtocolIso14443_3a, data);
    nfc_test_save_and_load(nfc_device);

    iso14443_3a_free(data);
    nfc_device_free(nfc_device);
}

static void nfc_file_test_with_generator(NfcDataGeneratorType type) {
    NfcDevice* nfc_device_ref = nfc_device_alloc();

    nfc_data_generator_fill_data(type, nfc_device_ref);
    nfc_test_save_and_load(nfc_device_ref);

    nfc_device_free(nfc_device_ref);
}

MU_TEST(iso14443_3a_4b_file_test) {
    iso14443_3a_file_test(4);
}

MU_TEST(iso14443_3a_7b_file_test) {
    iso14443_3a_file_test(7);
}

MU_TEST(mf_ultralight_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfUltralight);
}

MU_TEST(mf_ultralight_ev1_11_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfUltralightEV1_11);
}

MU_TEST(mf_ultralight_ev1_h11_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfUltralightEV1_H11);
}

MU_TEST(mf_ultralight_ev1_21_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfUltralightEV1_21);
}

MU_TEST(mf_ultralight_ev1_h21_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfUltralightEV1_H21);
}

MU_TEST(mf_ultralight_ntag_203_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAG203);
}

MU_TEST(mf_ultralight_ntag_213_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAG213);
}

MU_TEST(mf_ultralight_ntag_215_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAG215);
}

MU_TEST(mf_ultralight_ntag_216_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAG216);
}

MU_TEST(mf_ultralight_ntag_i2c_1k_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAGI2C1k);
}

MU_TEST(mf_ultralight_ntag_i2c_2k_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAGI2C2k);
}

MU_TEST(mf_ultralight_ntag_i2c_plus_1k_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAGI2CPlus1k);
}

MU_TEST(mf_ultralight_ntag_i2c_plus_2k_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeNTAGI2CPlus2k);
}

MU_TEST(mf_classic_mini_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfClassicMini);
}

MU_TEST(mf_classic_1k_4b_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfClassic1k_4b);
}

MU_TEST(mf_classic_1k_7b_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfClassic1k_7b);
}

MU_TEST(mf_classic_4k_4b_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfClassic4k_4b);
}

MU_TEST(mf_classic_4k_7b_file_test) {
    nfc_file_test_with_generator(NfcDataGeneratorTypeMfClassic4k_7b);
}

MU_TEST(iso14443_3a_reader) {
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    Iso14443_3aData iso14443_3a_listener_data = {
        .uid_len = 7,
        .uid = {0x04, 0x51, 0x5C, 0xFA, 0x6F, 0x73, 0x81},
        .atqa = {0x44, 0x00},
        .sak = 0x00,
    };
    NfcListener* iso3_listener =
        nfc_listener_alloc(listener, NfcProtocolIso14443_3a, &iso14443_3a_listener_data);
    nfc_listener_start(iso3_listener, NULL, NULL);

    Iso14443_3aData iso14443_3a_poller_data = {};
    mu_assert(
        iso14443_3a_poller_sync_read(poller, &iso14443_3a_poller_data) == Iso14443_3aErrorNone,
        "iso14443_3a_poller_sync_read() failed");

    nfc_listener_stop(iso3_listener);
    mu_assert(
        iso14443_3a_is_equal(&iso14443_3a_poller_data, &iso14443_3a_listener_data),
        "Data not matches");

    nfc_listener_free(iso3_listener);
    nfc_free(listener);
    nfc_free(poller);
}

static void mf_ultralight_reader_test(const char* path) {
    FURI_LOG_I(TAG, "Testing file: %s", path);
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDevice* nfc_device = nfc_device_alloc();
    mu_assert(nfc_device_load(nfc_device, path), "nfc_device_load() failed\r\n");

    MfUltralightData* data =
        (MfUltralightData*)nfc_device_get_data(nfc_device, NfcProtocolMfUltralight);

    uint32_t features = mf_ultralight_get_feature_support_set(data->type);
    bool pwd_supported =
        mf_ultralight_support_feature(features, MfUltralightFeatureSupportPasswordAuth);
    uint8_t pwd_num = mf_ultralight_get_pwd_page_num(data->type);
    const uint8_t zero_pwd[4] = {0, 0, 0, 0};

    if(pwd_supported && !memcmp(data->page[pwd_num].data, zero_pwd, sizeof(zero_pwd))) {
        data->pages_read -= 2;
    }

    NfcListener* mfu_listener = nfc_listener_alloc(listener, NfcProtocolMfUltralight, data);

    nfc_listener_start(mfu_listener, NULL, NULL);

    MfUltralightData* mfu_data = mf_ultralight_alloc();
    MfUltralightError error = mf_ultralight_poller_sync_read_card(poller, mfu_data, NULL);
    mu_assert(error == MfUltralightErrorNone, "mf_ultralight_poller_sync_read_card() failed");

    nfc_listener_stop(mfu_listener);
    nfc_listener_free(mfu_listener);

    MfUltralightData* mfu_other_data =
        (MfUltralightData*)nfc_device_get_data(nfc_device, NfcProtocolMfUltralight);
    mu_assert(mf_ultralight_is_equal(mfu_data, mfu_other_data), "Data mismatch");

    mf_ultralight_free(mfu_data);
    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

MU_TEST(mf_ultralight_11_reader) {
    mf_ultralight_reader_test(EXT_PATH("unit_tests/nfc/Ultralight_11.nfc"));
}

MU_TEST(mf_ultralight_21_reader) {
    mf_ultralight_reader_test(EXT_PATH("unit_tests/nfc/Ultralight_21.nfc"));
}

MU_TEST(mf_ultralight_c_reader) {
    mf_ultralight_reader_test(EXT_PATH("unit_tests/nfc/Ultralight_C.nfc"));
}

MU_TEST(ntag_215_reader) {
    mf_ultralight_reader_test(EXT_PATH("unit_tests/nfc/Ntag215.nfc"));
}

MU_TEST(ntag_216_reader) {
    mf_ultralight_reader_test(EXT_PATH("unit_tests/nfc/Ntag216.nfc"));
}

MU_TEST(ntag_213_locked_reader) {
    FURI_LOG_I(TAG, "Testing Ntag215 locked file");
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDeviceData* nfc_device = nfc_device_alloc();
    mu_assert(
        nfc_device_load(nfc_device, EXT_PATH("unit_tests/nfc/Ntag213_locked.nfc")),
        "nfc_device_load() failed\r\n");

    NfcListener* mfu_listener = nfc_listener_alloc(
        listener,
        NfcProtocolMfUltralight,
        nfc_device_get_data(nfc_device, NfcProtocolMfUltralight));
    nfc_listener_start(mfu_listener, NULL, NULL);

    MfUltralightData* mfu_data = mf_ultralight_alloc();
    MfUltralightError error = mf_ultralight_poller_sync_read_card(poller, mfu_data, NULL);
    mu_assert(error == MfUltralightErrorNone, "mf_ultralight_poller_sync_read_card() failed");

    nfc_listener_stop(mfu_listener);
    nfc_listener_free(mfu_listener);

    MfUltralightConfigPages* config = NULL;
    const MfUltralightData* mfu_ref_data =
        nfc_device_get_data(nfc_device, NfcProtocolMfUltralight);
    mu_assert(
        mf_ultralight_get_config_page(mfu_ref_data, &config),
        "mf_ultralight_get_config_page() failed");
    uint16_t pages_locked = config->auth0;

    mu_assert(mfu_data->pages_read == pages_locked, "Unexpected pages read");

    mf_ultralight_free(mfu_data);
    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

static void mf_ultralight_write(void) {
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDevice* nfc_device = nfc_device_alloc();
    nfc_data_generator_fill_data(NfcDataGeneratorTypeMfUltralightEV1_21, nfc_device);

    NfcListener* mfu_listener = nfc_listener_alloc(
        listener,
        NfcProtocolMfUltralight,
        nfc_device_get_data(nfc_device, NfcProtocolMfUltralight));
    nfc_listener_start(mfu_listener, NULL, NULL);

    MfUltralightData* mfu_data = mf_ultralight_alloc();

    // Initial read
    MfUltralightError error = mf_ultralight_poller_sync_read_card(poller, mfu_data, NULL);
    mu_assert(error == MfUltralightErrorNone, "mf_ultralight_poller_sync_read_card() failed");

    mu_assert(
        mf_ultralight_is_equal(mfu_data, nfc_device_get_data(nfc_device, NfcProtocolMfUltralight)),
        "Data not matches");

    // Write random data
    for(size_t i = 5; i < 15; i++) {
        MfUltralightPage page = {};
        FURI_LOG_D(TAG, "Writing page %d", i);
        furi_hal_random_fill_buf(page.data, sizeof(MfUltralightPage));
        mfu_data->page[i] = page;
        error = mf_ultralight_poller_sync_write_page(poller, i, &page);
        mu_assert(error == MfUltralightErrorNone, "mf_ultralight_poller_sync_write_page() failed");
    }

    // Verification read
    error = mf_ultralight_poller_sync_read_card(poller, mfu_data, NULL);
    mu_assert(error == MfUltralightErrorNone, "mf_ultralight_poller_sync_read_card() failed");

    nfc_listener_stop(mfu_listener);
    const MfUltralightData* mfu_listener_data =
        nfc_listener_get_data(mfu_listener, NfcProtocolMfUltralight);

    mu_assert(mf_ultralight_is_equal(mfu_data, mfu_listener_data), "Data not matches");

    nfc_listener_free(mfu_listener);
    mf_ultralight_free(mfu_data);
    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

static void mf_classic_reader(void) {
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDevice* nfc_device = nfc_device_alloc();
    nfc_data_generator_fill_data(NfcDataGeneratorTypeMfClassic4k_7b, nfc_device);
    NfcListener* mfc_listener = nfc_listener_alloc(
        listener, NfcProtocolMfClassic, nfc_device_get_data(nfc_device, NfcProtocolMfClassic));
    nfc_listener_start(mfc_listener, NULL, NULL);

    MfClassicBlock block = {};
    MfClassicKey key = {.data = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

    mf_classic_poller_sync_read_block(poller, 0, &key, MfClassicKeyTypeA, &block);

    nfc_listener_stop(mfc_listener);
    nfc_listener_free(mfc_listener);

    const MfClassicData* mfc_data = nfc_device_get_data(nfc_device, NfcProtocolMfClassic);
    mu_assert(memcmp(&mfc_data->block[0], &block, sizeof(MfClassicBlock)) == 0, "Data mismatch");

    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

static void mf_classic_write(void) {
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDevice* nfc_device = nfc_device_alloc();
    nfc_data_generator_fill_data(NfcDataGeneratorTypeMfClassic4k_7b, nfc_device);
    NfcListener* mfc_listener = nfc_listener_alloc(
        listener, NfcProtocolMfClassic, nfc_device_get_data(nfc_device, NfcProtocolMfClassic));
    nfc_listener_start(mfc_listener, NULL, NULL);

    MfClassicBlock block_write = {};
    MfClassicBlock block_read = {};
    furi_hal_random_fill_buf(block_write.data, sizeof(MfClassicBlock));
    MfClassicKey key = {.data = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

    mf_classic_poller_sync_write_block(poller, 1, &key, MfClassicKeyTypeA, &block_write);
    mf_classic_poller_sync_read_block(poller, 1, &key, MfClassicKeyTypeA, &block_read);

    nfc_listener_stop(mfc_listener);
    nfc_listener_free(mfc_listener);

    mu_assert(memcmp(&block_read, &block_write, sizeof(MfClassicBlock)) == 0, "Data mismatch");

    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

static void mf_classic_value_block(void) {
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDevice* nfc_device = nfc_device_alloc();
    nfc_data_generator_fill_data(NfcDataGeneratorTypeMfClassic4k_7b, nfc_device);
    NfcListener* mfc_listener = nfc_listener_alloc(
        listener, NfcProtocolMfClassic, nfc_device_get_data(nfc_device, NfcProtocolMfClassic));
    nfc_listener_start(mfc_listener, NULL, NULL);

    MfClassicKey key = {.data = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

    int32_t value = 228;
    MfClassicBlock block_write = {};
    mf_classic_value_to_block(value, 1, &block_write);

    MfClassicError error = MfClassicErrorNone;
    error = mf_classic_poller_sync_write_block(poller, 1, &key, MfClassicKeyTypeA, &block_write);
    mu_assert(error == MfClassicErrorNone, "Write failed");

    int32_t data = 200;
    int32_t new_value = 0;
    error =
        mf_classic_poller_sync_change_value(poller, 1, &key, MfClassicKeyTypeA, data, &new_value);
    mu_assert(error == MfClassicErrorNone, "Value increment failed");
    mu_assert(new_value == value + data, "Value not match");

    error =
        mf_classic_poller_sync_change_value(poller, 1, &key, MfClassicKeyTypeA, -data, &new_value);
    mu_assert(error == MfClassicErrorNone, "Value decrement failed");
    mu_assert(new_value == value, "Value not match");

    nfc_listener_stop(mfc_listener);
    nfc_listener_free(mfc_listener);
    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

NfcCommand mf_classic_poller_send_frame_callback(NfcGenericEventEx event, void* context) {
    furi_check(event.poller);
    furi_check(event.parent_event_data);
    furi_check(context);

    NfcCommand command = NfcCommandContinue;
    MfClassicPoller* instance = event.poller;
    NfcTestMfClassicSendFrameTest* frame_test = context;
    Iso14443_3aPollerEvent* iso3_event = event.parent_event_data;

    MfClassicError error = MfClassicErrorNone;
    if(iso3_event->type == Iso14443_3aPollerEventTypeReady) {
        if(frame_test->state == NfcTestMfClassicSendFrameTestStateAuth) {
            MfClassicKey key = {
                .data = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
            };
            error = mf_classic_poller_auth(instance, 0, &key, MfClassicKeyTypeA, NULL, false);
            frame_test->state = (error == MfClassicErrorNone) ?
                                    NfcTestMfClassicSendFrameTestStateReadBlock :
                                    NfcTestMfClassicSendFrameTestStateFail;
        } else if(frame_test->state == NfcTestMfClassicSendFrameTestStateReadBlock) {
            do {
                const uint8_t read_block_cmd[] = {
                    0x30,
                    0x01,
                    0x8b,
                    0xb9,
                };
                bit_buffer_copy_bytes(frame_test->tx_buf, read_block_cmd, sizeof(read_block_cmd));

                error = mf_classic_poller_send_encrypted_frame(
                    instance, frame_test->tx_buf, frame_test->rx_buf, 200000);
                if(error != MfClassicErrorNone) break;
                if(bit_buffer_get_size_bytes(frame_test->rx_buf) != 18) {
                    error = MfClassicErrorProtocol;
                    break;
                }

                const uint8_t* rx_data = bit_buffer_get_data(frame_test->rx_buf);
                const uint8_t rx_data_ref[16] = {0};
                if(memcmp(rx_data, rx_data_ref, sizeof(rx_data_ref)) != 0) {
                    error = MfClassicErrorProtocol;
                    break;
                }
            } while(false);

            frame_test->state = (error == MfClassicErrorNone) ?
                                    NfcTestMfClassicSendFrameTestStateSuccess :
                                    NfcTestMfClassicSendFrameTestStateFail;
        } else if(frame_test->state == NfcTestMfClassicSendFrameTestStateSuccess) {
            command = NfcCommandStop;
        } else if(frame_test->state == NfcTestMfClassicSendFrameTestStateFail) {
            command = NfcCommandStop;
        }
    } else {
        frame_test->state = NfcTestMfClassicSendFrameTestStateFail;
        command = NfcCommandStop;
    }

    if(command == NfcCommandStop) {
        furi_thread_flags_set(frame_test->thread_id, NFC_TEST_FLAG_WORKER_DONE);
    }

    return command;
}

MU_TEST(mf_classic_send_frame_test) {
    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDevice* nfc_device = nfc_device_alloc();
    nfc_data_generator_fill_data(NfcDataGeneratorTypeMfClassic4k_7b, nfc_device);
    NfcListener* mfc_listener = nfc_listener_alloc(
        listener, NfcProtocolMfClassic, nfc_device_get_data(nfc_device, NfcProtocolMfClassic));
    nfc_listener_start(mfc_listener, NULL, NULL);

    NfcPoller* mfc_poller = nfc_poller_alloc(poller, NfcProtocolMfClassic);
    NfcTestMfClassicSendFrameTest context = {
        .state = NfcTestMfClassicSendFrameTestStateAuth,
        .thread_id = furi_thread_get_current_id(),
        .tx_buf = bit_buffer_alloc(32),
        .rx_buf = bit_buffer_alloc(32),
    };
    nfc_poller_start_ex(mfc_poller, mf_classic_poller_send_frame_callback, &context);

    uint32_t flag =
        furi_thread_flags_wait(NFC_TEST_FLAG_WORKER_DONE, FuriFlagWaitAny, FuriWaitForever);
    mu_assert(flag == NFC_TEST_FLAG_WORKER_DONE, "Wrong thread flag");
    nfc_poller_stop(mfc_poller);
    nfc_poller_free(mfc_poller);

    mu_assert(
        context.state == NfcTestMfClassicSendFrameTestStateSuccess, "Wrong test state at the end");

    bit_buffer_free(context.tx_buf);
    bit_buffer_free(context.rx_buf);
    nfc_listener_stop(mfc_listener);
    nfc_listener_free(mfc_listener);
    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

MU_TEST(mf_classic_dict_test) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(storage_common_stat(storage, NFC_APP_MF_CLASSIC_DICT_UNIT_TEST_PATH, NULL) == FSE_OK) {
        mu_assert(
            storage_simply_remove(storage, NFC_APP_MF_CLASSIC_DICT_UNIT_TEST_PATH),
            "Remove test dict failed");
    }

    KeysDict* dict = keys_dict_alloc(
        NFC_APP_MF_CLASSIC_DICT_UNIT_TEST_PATH, KeysDictModeOpenAlways, sizeof(MfClassicKey));
    mu_assert(dict != NULL, "keys_dict_alloc() failed");

    size_t dict_keys_total = keys_dict_get_total_keys(dict);
    mu_assert(dict_keys_total == 0, "keys_dict_keys_total() failed");

    const uint32_t test_key_num = 30;
    MfClassicKey* key_arr_ref = malloc(test_key_num * sizeof(MfClassicKey));
    for(size_t i = 0; i < test_key_num; i++) {
        furi_hal_random_fill_buf(key_arr_ref[i].data, sizeof(MfClassicKey));
        mu_assert(
            keys_dict_add_key(dict, key_arr_ref[i].data, sizeof(MfClassicKey)), "add key failed");

        size_t dict_keys_total = keys_dict_get_total_keys(dict);
        mu_assert(dict_keys_total == (i + 1), "keys_dict_keys_total() failed");
    }

    keys_dict_free(dict);

    dict = keys_dict_alloc(
        NFC_APP_MF_CLASSIC_DICT_UNIT_TEST_PATH, KeysDictModeOpenAlways, sizeof(MfClassicKey));
    mu_assert(dict != NULL, "keys_dict_alloc() failed");

    dict_keys_total = keys_dict_get_total_keys(dict);
    mu_assert(dict_keys_total == test_key_num, "keys_dict_keys_total() failed");

    MfClassicKey key_dut = {};
    size_t key_idx = 0;
    while(keys_dict_get_next_key(dict, key_dut.data, sizeof(MfClassicKey))) {
        mu_assert(
            memcmp(key_arr_ref[key_idx].data, key_dut.data, sizeof(MfClassicKey)) == 0,
            "Loaded key data mismatch");
        key_idx++;
    }

    uint32_t delete_keys_idx[] = {1, 3, 9, 11, 19, 27};

    for(size_t i = 0; i < COUNT_OF(delete_keys_idx); i++) {
        MfClassicKey* key = &key_arr_ref[delete_keys_idx[i]];
        mu_assert(
            keys_dict_is_key_present(dict, key->data, sizeof(MfClassicKey)),
            "keys_dict_is_key_present() failed");
        mu_assert(
            keys_dict_delete_key(dict, key->data, sizeof(MfClassicKey)),
            "keys_dict_delete_key() failed");
    }

    dict_keys_total = keys_dict_get_total_keys(dict);
    mu_assert(
        dict_keys_total == test_key_num - COUNT_OF(delete_keys_idx),
        "keys_dict_keys_total() failed");

    keys_dict_free(dict);
    free(key_arr_ref);

    mu_assert(
        storage_simply_remove(storage, NFC_APP_MF_CLASSIC_DICT_UNIT_TEST_PATH),
        "Remove test dict failed");
}

static FelicaError
    felica_do_request_response(FelicaData* felica_data, const FelicaCardKey* card_key) {
    NfcDeviceData* nfc_device = nfc_device_alloc();

    FelicaError error = FelicaErrorNone;
    if(!nfc_device_load(nfc_device, EXT_PATH("unit_tests/nfc/Felica.nfc"))) {
        error = FelicaErrorNotPresent;
    } else {
        Nfc* poller = nfc_alloc();
        Nfc* listener = nfc_alloc();
        NfcListener* felica_listener = nfc_listener_alloc(
            listener, NfcProtocolFelica, nfc_device_get_data(nfc_device, NfcProtocolFelica));
        nfc_listener_start(felica_listener, NULL, NULL);

        error = felica_poller_sync_read(poller, felica_data, card_key);

        nfc_listener_stop(felica_listener);
        nfc_listener_free(felica_listener);

        nfc_free(listener);
        nfc_free(poller);
    }

    nfc_device_free(nfc_device);
    return error;
}

MU_TEST(felica_read) {
    FelicaData* felica_data = felica_alloc();
    FelicaError error = felica_do_request_response(felica_data, NULL);
    mu_assert(error == FelicaErrorNone, "felica_poller() failed");
    mu_assert(felica_data->data.fs.spad[4].SF1 == 0x01, "block[4].SF1 != 0x01");
    mu_assert(felica_data->data.fs.spad[4].SF2 == 0xB1, "block[4].SF2 != 0xB1");

    felica_free(felica_data);
}

// Minimal check of the FeliCa file format v3: no radio involved, just parsing
MU_TEST(felica_standard_file_test) {
    NfcDevice* nfc_device = nfc_device_alloc();
    mu_assert(
        nfc_device_load(nfc_device, EXT_PATH("unit_tests/nfc/Felica_Standard_with_keys.nfc")),
        "nfc_device_load() failed");

    const FelicaData* data = nfc_device_get_data(nfc_device, NfcProtocolFelica);
    mu_assert(data->workflow_type == FelicaStandard, "workflow_type != FelicaStandard");
    mu_assert(simple_array_get_count(data->systems) == 1, "system count != 1");

    const FelicaSystem* system = simple_array_cget(data->systems, 0);
    mu_assert(system->system_code == 0x0003, "system_code != 0x0003");
    mu_assert(system->key_version == 0x0001, "system key_version != 0x0001");

    mu_assert(simple_array_get_count(system->areas) == 1, "area count != 1");
    const FelicaArea* area = simple_array_cget(system->areas, 0);
    mu_assert(area->code == 0x0000, "area code != 0x0000");
    mu_assert(area->end_code == 0xFFFE, "area end_code != 0xFFFE");
    // Area 0000 is the system itself, so its key version mirrors the system one
    mu_assert(area->key_version == 0x0001, "area key_version != 0x0001");

    mu_assert(simple_array_get_count(system->services) == 2, "service count != 2");
    const FelicaService* service0 = simple_array_cget(system->services, 0);
    mu_assert(service0->code == 0x0009, "service[0] code != 0x0009");
    mu_assert(service0->key_version == 0x0003, "service[0] key_version != 0x0003");
    const FelicaService* service1 = simple_array_cget(system->services, 1);
    mu_assert(service1->code == 0x000B, "service[1] code != 0x000B");
    mu_assert(service1->key_version == 0x0004, "service[1] key_version != 0x0004");

    nfc_device_free(nfc_device);
}

// Version 2 files carry neither area end codes nor key versions
MU_TEST(felica_standard_v2_file_test) {
    NfcDevice* nfc_device = nfc_device_alloc();
    mu_assert(
        nfc_device_load(nfc_device, EXT_PATH("unit_tests/nfc/Felica_Standard_v2.nfc")),
        "nfc_device_load() failed");

    const FelicaData* data = nfc_device_get_data(nfc_device, NfcProtocolFelica);
    mu_assert(data->workflow_type == FelicaStandard, "workflow_type != FelicaStandard");
    mu_assert(simple_array_get_count(data->systems) == 1, "system count != 1");

    const FelicaSystem* system = simple_array_cget(data->systems, 0);
    mu_assert(system->system_code == 0x0003, "system_code != 0x0003");
    mu_assert(system->key_version == FELICA_KEY_VERSION_UNKNOWN, "system key_version is known");

    mu_assert(simple_array_get_count(system->areas) == 1, "area count != 1");
    const FelicaArea* area = simple_array_cget(system->areas, 0);
    mu_assert(area->code == 0x0000, "area code != 0x0000");
    mu_assert(area->end_code == FELICA_AREA_END_CODE_UNKNOWN, "area end_code is known");
    mu_assert(area->key_version == FELICA_KEY_VERSION_UNKNOWN, "area key_version is known");
    mu_assert(area->first_idx == 0, "area first_idx != 0");
    mu_assert(area->last_idx == 1, "area last_idx != 1");

    mu_assert(simple_array_get_count(system->services) == 2, "service count != 2");
    const FelicaService* service0 = simple_array_cget(system->services, 0);
    mu_assert(service0->code == 0x0009, "service[0] code != 0x0009");
    mu_assert(service0->key_version == FELICA_KEY_VERSION_UNKNOWN, "service[0] key_version known");
    const FelicaService* service1 = simple_array_cget(system->services, 1);
    mu_assert(service1->code == 0x000B, "service[1] code != 0x000B");
    mu_assert(service1->key_version == FELICA_KEY_VERSION_UNKNOWN, "service[1] key_version known");

    mu_assert(simple_array_get_count(system->public_blocks) == 2, "public block count != 2");

    nfc_device_free(nfc_device);
}

MU_TEST(felica_oversized_lite_file_test) {
    NfcDevice* nfc_device = nfc_device_alloc();
    mu_assert(
        !nfc_device_load(nfc_device, EXT_PATH("unit_tests/nfc/Felica_oversized_lite.nfc")),
        "oversized FeliCa Lite file was accepted");
    nfc_device_free(nfc_device);
}

MU_TEST(felica_standard_read) {
    NfcDeviceData* nfc_device = nfc_device_alloc();
    mu_assert(
        nfc_device_load(nfc_device, EXT_PATH("unit_tests/nfc/Felica_Standard_with_keys.nfc")),
        "nfc_device_load() failed");

    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();
    NfcListener* felica_listener = nfc_listener_alloc(
        listener, NfcProtocolFelica, nfc_device_get_data(nfc_device, NfcProtocolFelica));
    nfc_listener_start(felica_listener, NULL, NULL);

    FelicaData* felica_data = felica_alloc();
    FelicaError error = felica_poller_sync_read(poller, felica_data, NULL);

    nfc_listener_stop(felica_listener);
    nfc_listener_free(felica_listener);
    nfc_free(listener);
    nfc_free(poller);
    nfc_device_free(nfc_device);

    mu_assert(error == FelicaErrorNone, "felica_poller_sync_read() failed");
    mu_assert(simple_array_get_count(felica_data->systems) == 1, "system count != 1");

    const FelicaSystem* system = simple_array_cget(felica_data->systems, 0);
    mu_assert(system->key_version == 0x0001, "system key_version != 0x0001");
    mu_assert(simple_array_get_count(system->public_blocks) == 2, "public block count != 2");

    const FelicaPublicBlock* pb0 = simple_array_cget(system->public_blocks, 0);
    mu_assert(pb0->service_code == 0x0009, "block[0] service_code != 0x0009");
    mu_assert(pb0->block_idx == 0, "block[0] block_idx != 0");
    static const uint8_t expected_block0[FELICA_DATA_BLOCK_SIZE] = {
        0x01,
        0x02,
        0x03,
        0x04,
        0x05,
        0x06,
        0x07,
        0x08,
        0x09,
        0x0A,
        0x0B,
        0x0C,
        0x0D,
        0x0E,
        0x0F,
        0x10};
    mu_assert(
        memcmp(pb0->block.data, expected_block0, FELICA_DATA_BLOCK_SIZE) == 0,
        "block[0] data mismatch");

    const FelicaPublicBlock* pb1 = simple_array_cget(system->public_blocks, 1);
    mu_assert(pb1->service_code == 0x000B, "block[1] service_code != 0x000B");
    mu_assert(pb1->block_idx == 0, "block[1] block_idx != 0");
    static const uint8_t expected_block1[FELICA_DATA_BLOCK_SIZE] = {
        0xA1,
        0xA2,
        0xA3,
        0xA4,
        0xA5,
        0xA6,
        0xA7,
        0xA8,
        0xA9,
        0xAA,
        0xAB,
        0xAC,
        0xAD,
        0xAE,
        0xAF,
        0xB0};
    mu_assert(
        memcmp(pb1->block.data, expected_block1, FELICA_DATA_BLOCK_SIZE) == 0,
        "block[1] data mismatch");

    felica_free(felica_data);
}

MU_TEST(felica_read_auth) {
    FelicaData* felica_data = felica_alloc();
    FelicaCardKey card_key;
    memset(card_key.data, 0xFF, FELICA_DATA_BLOCK_SIZE);

    FelicaError error = felica_do_request_response(felica_data, &card_key);
    mu_assert(error == FelicaErrorNone, "felica_poller() failed");
    mu_assert(felica_data->data.fs.spad[4].SF1 == 0x00, "block[4].SF1 != 0x00");
    mu_assert(felica_data->data.fs.spad[4].SF2 == 0x00, "block[4].SF2 != 0x00");

    felica_free(felica_data);
}

MU_TEST(slix_file_with_capabilities_test) {
    NfcDevice* nfc_device_missed_cap = nfc_device_alloc();
    mu_assert(
        nfc_device_load(nfc_device_missed_cap, EXT_PATH("unit_tests/nfc/Slix_cap_missed.nfc")),
        "nfc_device_load() failed\r\n");

    NfcDevice* nfc_device_default_cap = nfc_device_alloc();
    mu_assert(
        nfc_device_load(nfc_device_default_cap, EXT_PATH("unit_tests/nfc/Slix_cap_default.nfc")),
        "nfc_device_load() failed\r\n");

    mu_assert(
        nfc_device_is_equal(nfc_device_missed_cap, nfc_device_default_cap),
        "nfc_device_is_equal() failed\r\n");

    nfc_device_free(nfc_device_default_cap);
    nfc_device_free(nfc_device_missed_cap);
}

NfcCommand slix_poller_set_password_callback(NfcGenericEventEx event, void* context) {
    furi_check(event.poller);
    furi_check(event.parent_event_data);
    furi_check(context);

    NfcCommand command = NfcCommandContinue;
    Iso15693_3PollerEvent* iso15_event = event.parent_event_data;
    SlixPoller* poller = event.poller;
    NfcTestSlixPollerSetPasswordContext* slix_ctx = context;

    if(iso15_event->type == Iso15693_3PollerEventTypeReady) {
        iso15693_3_copy(
            poller->data->iso15693_3_data, iso15693_3_poller_get_data(poller->iso15693_3_poller));

        if(slix_ctx->state == NfcTestSlixPollerSetPasswordStateGetRandomNumber) {
            slix_ctx->error = slix_poller_get_random_number(poller, &slix_ctx->random_number);
            if(slix_ctx->error != SlixErrorNone) {
                furi_thread_flags_set(slix_ctx->thread_id, NFC_TEST_FLAG_WORKER_DONE);
                command = NfcCommandStop;
            } else {
                slix_ctx->state = NfcTestSlixPollerSetPasswordStateSetPassword;
            }
        } else if(slix_ctx->state == NfcTestSlixPollerSetPasswordStateSetPassword) {
            slix_ctx->error = slix_poller_set_password(
                poller, SlixPasswordTypeRead, slix_ctx->password, slix_ctx->random_number);
            furi_thread_flags_set(slix_ctx->thread_id, NFC_TEST_FLAG_WORKER_DONE);
            command = NfcCommandStop;
        }
    } else {
        slix_ctx->error = slix_process_iso15693_3_error(iso15_event->data->error);
        furi_thread_flags_set(slix_ctx->thread_id, NFC_TEST_FLAG_WORKER_DONE);
        command = NfcCommandStop;
    }

    return command;
}

static void slix_set_password_test(const char* file_path, SlixPassword pass, bool correct_pass) {
    FURI_LOG_I(TAG, "Testing file: %s", file_path);

    Nfc* poller = nfc_alloc();
    Nfc* listener = nfc_alloc();

    NfcDevice* nfc_device = nfc_device_alloc();
    mu_assert(nfc_device_load(nfc_device, file_path), "nfc_device_load() failed\r\n");

    const SlixData* slix_data = nfc_device_get_data(nfc_device, NfcProtocolSlix);
    NfcListener* slix_listener = nfc_listener_alloc(listener, NfcProtocolSlix, slix_data);
    nfc_listener_start(slix_listener, NULL, NULL);

    SlixCapabilities slix_capabilities = slix_data->capabilities;

    NfcPoller* slix_poller = nfc_poller_alloc(poller, NfcProtocolSlix);

    NfcTestSlixPollerSetPasswordContext slix_poller_context = {
        .thread_id = furi_thread_get_current_id(),
        .state = NfcTestSlixPollerSetPasswordStateGetRandomNumber,
        .password = pass,
        .error = SlixErrorNone,
    };

    nfc_poller_start_ex(slix_poller, slix_poller_set_password_callback, &slix_poller_context);

    uint32_t flag =
        furi_thread_flags_wait(NFC_TEST_FLAG_WORKER_DONE, FuriFlagWaitAny, FuriWaitForever);
    mu_assert(flag == NFC_TEST_FLAG_WORKER_DONE, "Wrong thread flag\r\n");

    nfc_poller_stop(slix_poller);
    nfc_poller_free(slix_poller);
    nfc_listener_stop(slix_listener);
    nfc_listener_free(slix_listener);

    mu_assert(
        slix_poller_context.state == NfcTestSlixPollerSetPasswordStateSetPassword,
        "Poller failed before setting password\r\n");

    if((slix_capabilities == SlixCapabilitiesAcceptAllPasswords) || (correct_pass)) {
        mu_assert(slix_poller_context.error == SlixErrorNone, "Failed to set password\r\n");
    } else {
        mu_assert(
            slix_poller_context.error == SlixErrorTimeout,
            "Must have received SlixErrorTimeout\r\n");
    }

    nfc_device_free(nfc_device);
    nfc_free(listener);
    nfc_free(poller);
}

MU_TEST(slix_set_password_default_cap_correct_pass) {
    slix_set_password_test(EXT_PATH("unit_tests/nfc/Slix_cap_default.nfc"), 0x00000000, true);
}

MU_TEST(slix_set_password_default_cap_incorrect_pass) {
    slix_set_password_test(EXT_PATH("unit_tests/nfc/Slix_cap_default.nfc"), 0x12341234, false);
}

MU_TEST(slix_set_password_access_all_passwords_cap) {
    slix_set_password_test(
        EXT_PATH("unit_tests/nfc/Slix_cap_accept_all_pass.nfc"), 0x12341234, false);
}

// AES-CMAC known-answer vectors from RFC 4493 (Appendix D.1), NIST AES-128 key. Exercises
// the empty / single-complete-block / partial-final / multi-block paths of the CMAC plus the
// cmac8 odd-index truncation used by MIFARE Plus SL3.
MU_TEST(mf_plus_crypto_cmac_rfc4493) {
    const uint8_t key[16] = {
        0x2b,
        0x7e,
        0x15,
        0x16,
        0x28,
        0xae,
        0xd2,
        0xa6,
        0xab,
        0xf7,
        0x15,
        0x88,
        0x09,
        0xcf,
        0x4f,
        0x3c};
    const uint8_t msg[64] = {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e,
                             0x11, 0x73, 0x93, 0x17, 0x2a, 0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03,
                             0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51, 0x30,
                             0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19,
                             0x1a, 0x0a, 0x52, 0xef, 0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b,
                             0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10};
    const uint8_t exp0[16] = {
        0xbb,
        0x1d,
        0x69,
        0x29,
        0xe9,
        0x59,
        0x37,
        0x28,
        0x7f,
        0xa3,
        0x7d,
        0x12,
        0x9b,
        0x75,
        0x67,
        0x46};
    const uint8_t exp16[16] = {
        0x07,
        0x0a,
        0x16,
        0xb4,
        0x6b,
        0x4d,
        0x41,
        0x44,
        0xf7,
        0x9b,
        0xdd,
        0x9d,
        0xd0,
        0x4a,
        0x28,
        0x7c};
    const uint8_t exp40[16] = {
        0xdf,
        0xa6,
        0x67,
        0x47,
        0xde,
        0x9a,
        0xe6,
        0x30,
        0x30,
        0xca,
        0x32,
        0x61,
        0x14,
        0x97,
        0xc8,
        0x27};
    const uint8_t exp64[16] = {
        0x51,
        0xf0,
        0xbe,
        0xbf,
        0x7e,
        0x3b,
        0x9d,
        0x92,
        0xfc,
        0x49,
        0x74,
        0x17,
        0x79,
        0x36,
        0x3c,
        0xfe};
    uint8_t mac[16];

    mf_plus_crypto_cmac(key, msg, 0, mac);
    mu_assert(memcmp(mac, exp0, 16) == 0, "CMAC RFC4493 len=0 mismatch");
    mf_plus_crypto_cmac(key, msg, 16, mac);
    mu_assert(memcmp(mac, exp16, 16) == 0, "CMAC RFC4493 len=16 mismatch");
    mf_plus_crypto_cmac(key, msg, 40, mac);
    mu_assert(memcmp(mac, exp40, 16) == 0, "CMAC RFC4493 len=40 mismatch");
    mf_plus_crypto_cmac(key, msg, 64, mac);
    mu_assert(memcmp(mac, exp64, 16) == 0, "CMAC RFC4493 len=64 mismatch");

    // cmac8 = the 8 odd-indexed bytes of the full CMAC (checked on the len=16 vector).
    const uint8_t exp8[8] = {0x0a, 0xb4, 0x4d, 0x44, 0x9b, 0x9d, 0x4a, 0x7c};
    uint8_t mac8[8];
    mf_plus_crypto_cmac8(key, msg, 16, mac8);
    mu_assert(memcmp(mac8, exp8, 8) == 0, "cmac8 RFC4493 len=16 mismatch");
}

// Locks the data-encryption IV byte layout to the reference fork's (U-Prox-validated) layout: both
// IVs pack the little-endian {R_ctr, W_ctr} word {R_lo, R_hi, W_lo, W_hi}. Read repeats it across
// IV[0..11] with TI at IV[12..15]; write puts TI at IV[0..3] then repeats it across IV[4..15].
MU_TEST(mf_plus_crypto_data_iv_layout) {
    const uint8_t ti[4] = {0xa1, 0xa2, 0xa3, 0xa4};
    uint8_t iv[16];

    // Read-only session (W_ctr == 0, R_ctr < 256) reduces to the hardware-validated single-low-byte
    // form: R_lo at [0]/[4]/[8], TI at [12..15].
    const uint8_t exp_read[16] = {
        0x05,
        0x00,
        0x00,
        0x00,
        0x05,
        0x00,
        0x00,
        0x00,
        0x05,
        0x00,
        0x00,
        0x00,
        0xa1,
        0xa2,
        0xa3,
        0xa4};
    mf_plus_crypto_build_read_iv(ti, 0x0005, 0x0000, iv);
    mu_assert(memcmp(iv, exp_read, 16) == 0, "read IV layout mismatch");

    // Write IV (R_ctr == 0, W_ctr == 0x0007): TI at [0..3], W_lo at [6]/[10]/[14].
    const uint8_t exp_write[16] = {
        0xa1,
        0xa2,
        0xa3,
        0xa4,
        0x00,
        0x00,
        0x07,
        0x00,
        0x00,
        0x00,
        0x07,
        0x00,
        0x00,
        0x00,
        0x07,
        0x00};
    mf_plus_crypto_build_write_iv(ti, 0x0000, 0x0007, iv);
    mu_assert(memcmp(iv, exp_write, 16) == 0, "write IV layout mismatch");

    // Both counters pack little-endian in full: distinct R_ctr/W_ctr bytes land at fixed positions
    // in the repeated {R_lo, R_hi, W_lo, W_hi} word.
    const uint8_t exp_read_full[16] = {
        0x02,
        0x01,
        0x04,
        0x03,
        0x02,
        0x01,
        0x04,
        0x03,
        0x02,
        0x01,
        0x04,
        0x03,
        0xa1,
        0xa2,
        0xa3,
        0xa4};
    mf_plus_crypto_build_read_iv(ti, 0x0102, 0x0304, iv);
    mu_assert(memcmp(iv, exp_read_full, 16) == 0, "read IV counter-word mismatch");

    const uint8_t exp_write_full[16] = {
        0xa1,
        0xa2,
        0xa3,
        0xa4,
        0x02,
        0x01,
        0x04,
        0x03,
        0x02,
        0x01,
        0x04,
        0x03,
        0x02,
        0x01,
        0x04,
        0x03};
    mf_plus_crypto_build_write_iv(ti, 0x0102, 0x0304, iv);
    mu_assert(memcmp(iv, exp_write_full, 16) == 0, "write IV counter-word mismatch");
}

MU_TEST_SUITE(nfc) {
    nfc_test_alloc();

    MU_RUN_TEST(iso14443_3a_reader);
    MU_RUN_TEST(mf_ultralight_11_reader);
    MU_RUN_TEST(mf_ultralight_21_reader);
    MU_RUN_TEST(ntag_215_reader);
    MU_RUN_TEST(ntag_216_reader);
    MU_RUN_TEST(ntag_213_locked_reader);
    MU_RUN_TEST(mf_ultralight_c_reader);

    MU_RUN_TEST(mf_ultralight_write);

    MU_RUN_TEST(iso14443_3a_4b_file_test);
    MU_RUN_TEST(iso14443_3a_7b_file_test);

    MU_RUN_TEST(mf_ultralight_file_test);
    MU_RUN_TEST(mf_ultralight_ev1_11_file_test);
    MU_RUN_TEST(mf_ultralight_ev1_h11_file_test);
    MU_RUN_TEST(mf_ultralight_ev1_21_file_test);
    MU_RUN_TEST(mf_ultralight_ev1_h21_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_203_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_213_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_215_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_216_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_i2c_1k_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_i2c_2k_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_i2c_plus_1k_file_test);
    MU_RUN_TEST(mf_ultralight_ntag_i2c_plus_2k_file_test);

    MU_RUN_TEST(mf_classic_mini_file_test);
    MU_RUN_TEST(mf_classic_1k_4b_file_test);
    MU_RUN_TEST(mf_classic_1k_7b_file_test);
    MU_RUN_TEST(mf_classic_4k_4b_file_test);
    MU_RUN_TEST(mf_classic_4k_7b_file_test);

    MU_RUN_TEST(mf_classic_reader);
    MU_RUN_TEST(mf_classic_write);
    MU_RUN_TEST(mf_classic_value_block);
    MU_RUN_TEST(mf_classic_send_frame_test);
    MU_RUN_TEST(mf_classic_dict_test);
    MU_RUN_TEST(felica_read);
    MU_RUN_TEST(felica_read_auth);
    MU_RUN_TEST(felica_standard_file_test);
    MU_RUN_TEST(felica_standard_v2_file_test);
    MU_RUN_TEST(felica_oversized_lite_file_test);
    MU_RUN_TEST(felica_standard_read);

    MU_RUN_TEST(slix_file_with_capabilities_test);
    MU_RUN_TEST(slix_set_password_default_cap_correct_pass);
    MU_RUN_TEST(slix_set_password_default_cap_incorrect_pass);
    MU_RUN_TEST(slix_set_password_access_all_passwords_cap);

    MU_RUN_TEST(mf_plus_crypto_cmac_rfc4493);
    MU_RUN_TEST(mf_plus_crypto_data_iv_layout);

    nfc_test_free();
}

int run_minunit_test_nfc(void) {
    MU_RUN_SUITE(nfc);
    return MU_EXIT_CODE;
}

TEST_API_DEFINE(run_minunit_test_nfc)

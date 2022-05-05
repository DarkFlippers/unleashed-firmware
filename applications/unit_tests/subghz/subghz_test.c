#include <furi.h>
#include <furi_hal.h>
#include "../minunit.h"
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/subghz_keystore.h>
#include <lib/subghz/subghz_file_encoder_worker.h>
#include <lib/subghz/protocols/registry.h>
#include <flipper_format/flipper_format_i.h>

#define TAG "SubGhz TEST"
#define KEYSTORE_DIR_NAME "/ext/subghz/assets/keeloq_mfcodes"
#define CAME_ATOMO_DIR_NAME "/ext/subghz/assets/came_atomo"
#define NICE_FLOR_S_DIR_NAME "/ext/subghz/assets/nice_flor_s"
#define TEST_RANDOM_DIR_NAME "/ext/unit_tests/subghz/test_random_raw.sub"
#define TEST_RANDOM_COUNT_PARSE 59
#define TEST_TIMEOUT 10000

static SubGhzEnvironment* environment_handler;
static SubGhzReceiver* receiver_handler;
//static SubGhzTransmitter* transmitter_handler;
static SubGhzFileEncoderWorker* file_worker_encoder_handler;
static uint16_t subghz_test_decoder_count = 0;

static void subghz_test_rx_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    string_t text;
    string_init(text);
    subghz_protocol_decoder_base_get_string(decoder_base, text);
    subghz_receiver_reset(receiver_handler);
    FURI_LOG_I(TAG, "\r\n%s", string_get_cstr(text));
    string_clear(text);
    subghz_test_decoder_count++;
}

static void subghz_test_init(void) {
    environment_handler = subghz_environment_alloc();
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        environment_handler, CAME_ATOMO_DIR_NAME);
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        environment_handler, NICE_FLOR_S_DIR_NAME);

    receiver_handler = subghz_receiver_alloc_init(environment_handler);
    subghz_receiver_set_filter(receiver_handler, SubGhzProtocolFlag_Decodable);
    subghz_receiver_set_rx_callback(receiver_handler, subghz_test_rx_callback, NULL);
}

static void subghz_test_deinit(void) {
    subghz_receiver_free(receiver_handler);
    subghz_environment_free(environment_handler);
}

static bool subghz_decode_test(const char* path, const char* name_decoder) {
    subghz_test_decoder_count = 0;
    uint32_t test_start = furi_hal_get_tick();

    SubGhzProtocolDecoderBase* decoder =
        subghz_receiver_search_decoder_base_by_name(receiver_handler, name_decoder);

    if(decoder) {
        file_worker_encoder_handler = subghz_file_encoder_worker_alloc();
        if(subghz_file_encoder_worker_start(file_worker_encoder_handler, path)) {
            //the worker needs a file in order to open and read part of the file
            osDelay(100);

            LevelDuration level_duration;
            while(furi_hal_get_tick() - test_start < TEST_TIMEOUT) {
                furi_hal_delay_us(
                    500); //you need to have time to read from the file from the SD card
                level_duration =
                    subghz_file_encoder_worker_get_level_duration(file_worker_encoder_handler);
                if(!level_duration_is_reset(level_duration)) {
                    bool level = level_duration_get_level(level_duration);
                    uint32_t duration = level_duration_get_duration(level_duration);
                    decoder->protocol->decoder->feed(decoder, level, duration);
                } else {
                    break;
                }
            }
            furi_hal_delay_ms(10);
        }
        if(subghz_file_encoder_worker_is_running(file_worker_encoder_handler)) {
            subghz_file_encoder_worker_stop(file_worker_encoder_handler);
        }
        subghz_file_encoder_worker_free(file_worker_encoder_handler);
    }
    FURI_LOG_I(TAG, "\r\n Decoder count parse \033[0;33m%d\033[0m ", subghz_test_decoder_count);
    if(furi_hal_get_tick() - test_start > TEST_TIMEOUT) {
        printf("\033[0;31mTest decoder %s ERROR TimeOut\033[0m\r\n", name_decoder);
        return false;
    } else {
        return subghz_test_decoder_count ? true : false;
    }
}

static bool subghz_decode_ramdom_test(const char* path) {
    subghz_test_decoder_count = 0;
    subghz_receiver_reset(receiver_handler);
    uint32_t test_start = furi_hal_get_tick();

    file_worker_encoder_handler = subghz_file_encoder_worker_alloc();
    if(subghz_file_encoder_worker_start(file_worker_encoder_handler, path)) {
        //the worker needs a file in order to open and read part of the file
        osDelay(100);

        LevelDuration level_duration;
        while(furi_hal_get_tick() - test_start < TEST_TIMEOUT * 10) {
            furi_hal_delay_us(500); //you need to have time to read from the file from the SD card
            level_duration =
                subghz_file_encoder_worker_get_level_duration(file_worker_encoder_handler);
            if(!level_duration_is_reset(level_duration)) {
                bool level = level_duration_get_level(level_duration);
                uint32_t duration = level_duration_get_duration(level_duration);
                subghz_receiver_decode(receiver_handler, level, duration);
            } else {
                break;
            }
        }
        furi_hal_delay_ms(10);
        if(subghz_file_encoder_worker_is_running(file_worker_encoder_handler)) {
            subghz_file_encoder_worker_stop(file_worker_encoder_handler);
        }
        subghz_file_encoder_worker_free(file_worker_encoder_handler);
    }
    FURI_LOG_I(TAG, "\r\n Decoder count parse \033[0;33m%d\033[0m ", subghz_test_decoder_count);
    if(furi_hal_get_tick() - test_start > TEST_TIMEOUT * 10) {
        printf("\033[0;31mRandom test ERROR TimeOut\033[0m\r\n");
        return false;
    } else if(subghz_test_decoder_count == TEST_RANDOM_COUNT_PARSE) {
        return true;
    } else {
        return false;
    }
}

static bool subghz_ecode_test(const char* path) {
    subghz_test_decoder_count = 0;
    uint32_t test_start = furi_hal_get_tick();
    string_t temp_str;
    string_init(temp_str);
    bool file_load = false;

    Storage* storage = furi_record_open("storage");
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    do {
        if(!flipper_format_file_open_existing(fff_data_file, path)) {
            FURI_LOG_E(TAG, "Error open file %s", path);
            break;
        }

        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            FURI_LOG_E(TAG, "Missing Preset");
            break;
        }

        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }
        file_load = true;
    } while(false);
    if(file_load) {
        SubGhzTransmitter* transmitter =
            subghz_transmitter_alloc_init(environment_handler, string_get_cstr(temp_str));
        subghz_transmitter_deserialize(transmitter, fff_data_file);

        SubGhzProtocolDecoderBase* decoder = subghz_receiver_search_decoder_base_by_name(
            receiver_handler, string_get_cstr(temp_str));

        if(decoder) {
            LevelDuration level_duration;
            while(furi_hal_get_tick() - test_start < TEST_TIMEOUT) {
                level_duration = subghz_transmitter_yield(transmitter);
                if(!level_duration_is_reset(level_duration)) {
                    bool level = level_duration_get_level(level_duration);
                    uint32_t duration = level_duration_get_duration(level_duration);
                    decoder->protocol->decoder->feed(decoder, level, duration);
                } else {
                    break;
                }
            }
            furi_hal_delay_ms(10);
        }
        subghz_transmitter_free(transmitter);
    }
    flipper_format_free(fff_data_file);
    FURI_LOG_I(TAG, "\r\n Decoder count parse \033[0;33m%d\033[0m ", subghz_test_decoder_count);
    if(furi_hal_get_tick() - test_start > TEST_TIMEOUT) {
        printf("\033[0;31mTest encoder %s ERROR TimeOut\033[0m\r\n", string_get_cstr(temp_str));
        subghz_test_decoder_count = 0;
    }
    string_clear(temp_str);

    return subghz_test_decoder_count ? true : false;
}

MU_TEST(subghz_keystore_test) {
    mu_assert(
        subghz_environment_load_keystore(environment_handler, KEYSTORE_DIR_NAME),
        "Test keystore error");
}

MU_TEST(subghz_decoder_came_atomo_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/came_atomo_raw.sub", SUBGHZ_PROTOCOL_CAME_ATOMO_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_CAME_ATOMO_NAME " error\r\n");
}

MU_TEST(subghz_decoder_came_test) {
    mu_assert(
        subghz_decode_test("/ext/unit_tests/subghz/came_raw.sub", SUBGHZ_PROTOCOL_CAME_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_CAME_NAME " error\r\n");
}

MU_TEST(subghz_decoder_came_twee_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/came_twee_raw.sub", SUBGHZ_PROTOCOL_CAME_TWEE_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_CAME_TWEE_NAME " error\r\n");
}

MU_TEST(subghz_decoder_faac_slh_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/faac_slh_raw.sub", SUBGHZ_PROTOCOL_FAAC_SLH_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_FAAC_SLH_NAME " error\r\n");
}

MU_TEST(subghz_decoder_gate_tx_test) {
    mu_assert(
        subghz_decode_test("/ext/unit_tests/subghz/gate_tx_raw.sub", SUBGHZ_PROTOCOL_GATE_TX_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_GATE_TX_NAME " error\r\n");
}

MU_TEST(subghz_decoder_hormann_hsm_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/hormann_hsm_raw.sub", SUBGHZ_PROTOCOL_HORMANN_HSM_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_HORMANN_HSM_NAME " error\r\n");
}

MU_TEST(subghz_decoder_ido_test) {
    mu_assert(
        subghz_decode_test("/ext/unit_tests/subghz/ido_117_111_raw.sub", SUBGHZ_PROTOCOL_IDO_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_IDO_NAME " error\r\n");
}

MU_TEST(subghz_decoder_keelog_test) {
    mu_assert(
        subghz_decode_test("/ext/unit_tests/subghz/doorhan_raw.sub", SUBGHZ_PROTOCOL_KEELOQ_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_KEELOQ_NAME " error\r\n");
}

MU_TEST(subghz_decoder_kia_seed_test) {
    mu_assert(
        subghz_decode_test("/ext/unit_tests/subghz/kia_seed_raw.sub", SUBGHZ_PROTOCOL_KIA_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_KIA_NAME " error\r\n");
}

MU_TEST(subghz_decoder_nero_radio_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/nero_radio_raw.sub", SUBGHZ_PROTOCOL_NERO_RADIO_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_NERO_RADIO_NAME " error\r\n");
}

MU_TEST(subghz_decoder_nero_sketch_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/nero_sketch_raw.sub", SUBGHZ_PROTOCOL_NERO_SKETCH_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_NERO_SKETCH_NAME " error\r\n");
}

MU_TEST(subghz_decoder_nice_flo_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/nice_flo_raw.sub", SUBGHZ_PROTOCOL_NICE_FLO_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_NICE_FLO_NAME " error\r\n");
}

MU_TEST(subghz_decoder_nice_flor_s_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/nice_flor_s_raw.sub", SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME " error\r\n");
}

MU_TEST(subghz_decoder_princeton_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/Princeton_raw.sub", SUBGHZ_PROTOCOL_PRINCETON_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_PRINCETON_NAME " error\r\n");
}

MU_TEST(subghz_decoder_scher_khan_magic_code_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/scher_khan_magic_code.sub", SUBGHZ_PROTOCOL_SCHER_KHAN_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_SCHER_KHAN_NAME " error\r\n");
}

MU_TEST(subghz_decoder_somfy_keytis_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/Somfy_keytis_raw.sub", SUBGHZ_PROTOCOL_SOMFY_KEYTIS_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_SOMFY_KEYTIS_NAME " error\r\n");
}

MU_TEST(subghz_decoder_somfy_telis_test) {
    mu_assert(
        subghz_decode_test(
            "/ext/unit_tests/subghz/somfy_telis_raw.sub", SUBGHZ_PROTOCOL_SOMFY_TELIS_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_SOMFY_TELIS_NAME " error\r\n");
}

MU_TEST(subghz_decoder_star_line_test) {
    mu_assert(
        subghz_decode_test("/ext/unit_tests/subghz/cenmax_raw.sub", SUBGHZ_PROTOCOL_STAR_LINE_NAME),
        "Test decoder " SUBGHZ_PROTOCOL_STAR_LINE_NAME " error\r\n");
}

MU_TEST(subghz_ecoder_princeton_test) {
    mu_assert(
        subghz_ecode_test("/ext/unit_tests/subghz/princeton.sub"),
        "Test encoder " SUBGHZ_PROTOCOL_PRINCETON_NAME " error\r\n");
}

MU_TEST(subghz_ecoder_came_test) {
    mu_assert(
        subghz_ecode_test("/ext/unit_tests/subghz/came.sub"),
        "Test encoder " SUBGHZ_PROTOCOL_CAME_NAME " error\r\n");
}

MU_TEST(subghz_ecoder_came_twee_test) {
    mu_assert(
        subghz_ecode_test("/ext/unit_tests/subghz/came_twee.sub"),
        "Test encoder " SUBGHZ_PROTOCOL_CAME_TWEE_NAME " error\r\n");
}

MU_TEST(subghz_ecoder_gate_tx_test) {
    mu_assert(
        subghz_ecode_test("/ext/unit_tests/subghz/gate_tx.sub"),
        "Test encoder " SUBGHZ_PROTOCOL_GATE_TX_NAME " error\r\n");
}

MU_TEST(subghz_ecoder_nice_flo_test) {
    mu_assert(
        subghz_ecode_test("/ext/unit_tests/subghz/nice_flo.sub"),
        "Test encoder " SUBGHZ_PROTOCOL_NICE_FLO_NAME " error\r\n");
}

MU_TEST(subghz_ecoder_keelog_test) {
    mu_assert(
        subghz_ecode_test("/ext/unit_tests/subghz/doorhan.sub"),
        "Test encoder " SUBGHZ_PROTOCOL_KEELOQ_NAME " error\r\n");
}

MU_TEST(subghz_random_test) {
    mu_assert(subghz_decode_ramdom_test(TEST_RANDOM_DIR_NAME), "Random test error\r\n");
}

MU_TEST_SUITE(subghz) {
    //MU_SUITE_CONFIGURE(&subghz_test_init, &subghz_test_deinit);

    subghz_test_init();
    MU_RUN_TEST(subghz_keystore_test);

    MU_RUN_TEST(subghz_decoder_came_atomo_test);
    MU_RUN_TEST(subghz_decoder_came_test);
    MU_RUN_TEST(subghz_decoder_came_twee_test);
    MU_RUN_TEST(subghz_decoder_faac_slh_test);
    MU_RUN_TEST(subghz_decoder_gate_tx_test);
    MU_RUN_TEST(subghz_decoder_hormann_hsm_test);
    MU_RUN_TEST(subghz_decoder_ido_test);
    MU_RUN_TEST(subghz_decoder_keelog_test);
    MU_RUN_TEST(subghz_decoder_kia_seed_test);
    MU_RUN_TEST(subghz_decoder_nero_radio_test);
    MU_RUN_TEST(subghz_decoder_nero_sketch_test);
    MU_RUN_TEST(subghz_decoder_nice_flo_test);
    MU_RUN_TEST(subghz_decoder_nice_flor_s_test);
    MU_RUN_TEST(subghz_decoder_princeton_test);
    MU_RUN_TEST(subghz_decoder_scher_khan_magic_code_test);
    MU_RUN_TEST(subghz_decoder_somfy_keytis_test);
    MU_RUN_TEST(subghz_decoder_somfy_telis_test);
    MU_RUN_TEST(subghz_decoder_star_line_test);

    MU_RUN_TEST(subghz_ecoder_princeton_test);
    MU_RUN_TEST(subghz_ecoder_came_test);
    MU_RUN_TEST(subghz_ecoder_came_twee_test);
    MU_RUN_TEST(subghz_ecoder_gate_tx_test);
    MU_RUN_TEST(subghz_ecoder_nice_flo_test);
    MU_RUN_TEST(subghz_ecoder_keelog_test);

    MU_RUN_TEST(subghz_random_test);
    subghz_test_deinit();
}

int run_minunit_test_subghz() {
    MU_RUN_SUITE(subghz);
    return MU_EXIT_CODE;
}

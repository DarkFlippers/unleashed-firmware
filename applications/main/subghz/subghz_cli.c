#include "subghz_cli.h"

#include <furi.h>
#include <furi_hal.h>

#include <lib/toolbox/args.h>
#include <lib/subghz/subghz_keystore.h>

#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/subghz_file_encoder_worker.h>
#include <lib/subghz/protocols/protocol_items.h>

#include "helpers/subghz_chat.h"

#include <notification/notification_messages.h>
#include <flipper_format/flipper_format_i.h>

#include <flipper.pb.h>
#include <pb_decode.h>

#define SUBGHZ_FREQUENCY_RANGE_STR \
    "299999755...348000000 or 386999938...464000000 or 778999847...928000000"

#define SUBGHZ_REGION_FILENAME "/int/.region_data"

void subghz_cli_command_tx_carrier(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    uint32_t frequency = 433920000;

    if(furi_string_size(args)) {
        int ret = sscanf(furi_string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz tx_carrier", "<Frequency: in Hz>", furi_string_get_cstr(args));
            return;
        }
        if(!furi_hal_subghz_is_frequency_valid(frequency)) {
            printf(
                "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    frequency = furi_hal_subghz_set_frequency_and_path(frequency);

    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_cc1101_g0, true);

    furi_hal_power_suppress_charge_enter();

    if(furi_hal_subghz_tx()) {
        printf("Transmitting at frequency %lu Hz\r\n", frequency);
        printf("Press CTRL+C to stop\r\n");
        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_ms(250);
        }
    } else {
        printf("This frequency can only be used for RX in your region\r\n");
    }

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();
}

void subghz_cli_command_rx_carrier(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    uint32_t frequency = 433920000;

    if(furi_string_size(args)) {
        int ret = sscanf(furi_string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz rx_carrier", "<Frequency: in Hz>", furi_string_get_cstr(args));
            return;
        }
        if(!furi_hal_subghz_is_frequency_valid(frequency)) {
            printf(
                "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    frequency = furi_hal_subghz_set_frequency_and_path(frequency);
    printf("Receiving at frequency %lu Hz\r\n", frequency);
    printf("Press CTRL+C to stop\r\n");

    furi_hal_power_suppress_charge_enter();

    furi_hal_subghz_rx();

    while(!cli_cmd_interrupt_received(cli)) {
        furi_delay_ms(250);
        printf("RSSI: %03.1fdbm\r", (double)furi_hal_subghz_get_rssi());
        fflush(stdout);
    }

    furi_hal_power_suppress_charge_exit();

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
}

void subghz_cli_command_tx(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    uint32_t frequency = 433920000;
    uint32_t key = 0x0074BADE;
    uint32_t repeat = 10;
    uint32_t te = 403;

    if(furi_string_size(args)) {
        int ret =
            sscanf(furi_string_get_cstr(args), "%lx %lu %lu %lu", &key, &frequency, &te, &repeat);
        if(ret != 4) {
            printf(
                "sscanf returned %d, key: %lx, frequency: %lu, te:%lu, repeat: %lu\r\n",
                ret,
                key,
                frequency,
                te,
                repeat);
            cli_print_usage(
                "subghz tx",
                "<3 Byte Key: in hex> <Frequency: in Hz> <Te us> <Repeat count>",
                furi_string_get_cstr(args));
            return;
        }
        if(!furi_hal_subghz_is_frequency_valid(frequency)) {
            printf(
                "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    printf(
        "Transmitting at %lu, key %lx, te %lu, repeat %lu. Press CTRL+C to stop\r\n",
        frequency,
        key,
        te,
        repeat);

    FuriString* flipper_format_string = furi_string_alloc_printf(
        "Protocol: Princeton\n"
        "Bit: 24\n"
        "Key: 00 00 00 00 00 %02X %02X %02X\n"
        "TE: %lu\n"
        "Repeat: %lu\n",
        (uint8_t)((key >> 16) & 0xFFU),
        (uint8_t)((key >> 8) & 0xFFU),
        (uint8_t)(key & 0xFFU),
        te,
        repeat);
    FlipperFormat* flipper_format = flipper_format_string_alloc();
    Stream* stream = flipper_format_get_raw_stream(flipper_format);
    stream_clean(stream);
    stream_write_cstring(stream, furi_string_get_cstr(flipper_format_string));

    SubGhzEnvironment* environment = subghz_environment_alloc();
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);

    SubGhzTransmitter* transmitter = subghz_transmitter_alloc_init(environment, "Princeton");
    subghz_transmitter_deserialize(transmitter, flipper_format);

    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    frequency = furi_hal_subghz_set_frequency_and_path(frequency);

    furi_hal_power_suppress_charge_enter();

    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, transmitter);

    while(!(furi_hal_subghz_is_async_tx_complete() || cli_cmd_interrupt_received(cli))) {
        printf(".");
        fflush(stdout);
        furi_delay_ms(333);
    }
    furi_hal_subghz_stop_async_tx();
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();

    flipper_format_free(flipper_format);
    subghz_transmitter_free(transmitter);
    subghz_environment_free(environment);
}

typedef struct {
    volatile bool overrun;
    FuriStreamBuffer* stream;
    size_t packet_count;
} SubGhzCliCommandRx;

static void subghz_cli_command_rx_capture_callback(bool level, uint32_t duration, void* context) {
    SubGhzCliCommandRx* instance = context;

    LevelDuration level_duration = level_duration_make(level, duration);
    if(instance->overrun) {
        instance->overrun = false;
        level_duration = level_duration_reset();
    }
    size_t ret =
        furi_stream_buffer_send(instance->stream, &level_duration, sizeof(LevelDuration), 0);
    if(sizeof(LevelDuration) != ret) instance->overrun = true;
}

static void subghz_cli_command_rx_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    SubGhzCliCommandRx* instance = context;
    instance->packet_count++;

    FuriString* text;
    text = furi_string_alloc();
    subghz_protocol_decoder_base_get_string(decoder_base, text);
    subghz_receiver_reset(receiver);
    printf("%s", furi_string_get_cstr(text));
    furi_string_free(text);
}

void subghz_cli_command_rx(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    uint32_t frequency = 433920000;

    if(furi_string_size(args)) {
        int ret = sscanf(furi_string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz rx", "<Frequency: in Hz>", furi_string_get_cstr(args));
            return;
        }
        if(!furi_hal_subghz_is_frequency_valid(frequency)) {
            printf(
                "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    // Allocate context and buffers
    SubGhzCliCommandRx* instance = malloc(sizeof(SubGhzCliCommandRx));
    instance->stream =
        furi_stream_buffer_alloc(sizeof(LevelDuration) * 1024, sizeof(LevelDuration));
    furi_check(instance->stream);

    SubGhzEnvironment* environment = subghz_environment_alloc();
    subghz_environment_load_keystore(environment, EXT_PATH("subghz/assets/keeloq_mfcodes"));
    subghz_environment_load_keystore(environment, EXT_PATH("subghz/assets/keeloq_mfcodes_user"));
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        environment, EXT_PATH("subghz/assets/came_atomo"));
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        environment, EXT_PATH("subghz/assets/alutech_at_4n"));
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        environment, EXT_PATH("subghz/assets/nice_flor_s"));
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);

    SubGhzReceiver* receiver = subghz_receiver_alloc_init(environment);
    subghz_receiver_set_filter(receiver, SubGhzProtocolFlag_Decodable);
    subghz_receiver_set_rx_callback(receiver, subghz_cli_command_rx_callback, instance);

    // Configure radio
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    frequency = furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    furi_hal_power_suppress_charge_enter();

    // Prepare and start RX
    furi_hal_subghz_start_async_rx(subghz_cli_command_rx_capture_callback, instance);

    // Wait for packets to arrive
    printf("Listening at %lu. Press CTRL+C to stop\r\n", frequency);
    LevelDuration level_duration;
    while(!cli_cmd_interrupt_received(cli)) {
        int ret = furi_stream_buffer_receive(
            instance->stream, &level_duration, sizeof(LevelDuration), 10);
        if(ret == sizeof(LevelDuration)) {
            if(level_duration_is_reset(level_duration)) {
                printf(".");
                subghz_receiver_reset(receiver);
            } else {
                bool level = level_duration_get_level(level_duration);
                uint32_t duration = level_duration_get_duration(level_duration);
                subghz_receiver_decode(receiver, level, duration);
            }
        }
    }

    // Shutdown radio
    furi_hal_subghz_stop_async_rx();
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();

    printf("\r\nPackets received %zu\r\n", instance->packet_count);

    // Cleanup
    subghz_receiver_free(receiver);
    subghz_environment_free(environment);
    furi_stream_buffer_free(instance->stream);
    free(instance);
}

void subghz_cli_command_rx_raw(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    uint32_t frequency = 433920000;

    if(furi_string_size(args)) {
        int ret = sscanf(furi_string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz rx", "<Frequency: in Hz>", furi_string_get_cstr(args));
            return;
        }
        if(!furi_hal_subghz_is_frequency_valid(frequency)) {
            printf(
                "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    // Allocate context and buffers
    SubGhzCliCommandRx* instance = malloc(sizeof(SubGhzCliCommandRx));
    instance->stream =
        furi_stream_buffer_alloc(sizeof(LevelDuration) * 1024, sizeof(LevelDuration));
    furi_check(instance->stream);

    // Configure radio
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok270Async);
    frequency = furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    furi_hal_power_suppress_charge_enter();

    // Prepare and start RX
    furi_hal_subghz_start_async_rx(subghz_cli_command_rx_capture_callback, instance);

    // Wait for packets to arrive
    printf("Listening at %lu. Press CTRL+C to stop\r\n", frequency);
    LevelDuration level_duration;
    size_t counter = 0;
    while(!cli_cmd_interrupt_received(cli)) {
        int ret = furi_stream_buffer_receive(
            instance->stream, &level_duration, sizeof(LevelDuration), 10);
        if(ret == 0) {
            continue;
        }
        if(ret != sizeof(LevelDuration)) {
            puts("stream corrupt");
            break;
        }
        if(level_duration_is_reset(level_duration)) {
            puts(". ");
        } else {
            bool level = level_duration_get_level(level_duration);
            uint32_t duration = level_duration_get_duration(level_duration);
            printf("%c%lu ", level ? '+' : '-', duration);
        }
        furi_thread_stdout_flush();
        counter++;
        if(counter > 255) {
            puts("\r\n");
            counter = 0;
        }
    }

    // Shutdown radio
    furi_hal_subghz_stop_async_rx();
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();

    // Cleanup
    furi_stream_buffer_free(instance->stream);
    free(instance);
}
void subghz_cli_command_decode_raw(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* file_name;
    file_name = furi_string_alloc();
    furi_string_set(file_name, ANY_PATH("subghz/test.sub"));

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    uint32_t temp_data32;
    bool check_file = false;

    do {
        if(furi_string_size(args)) {
            if(!args_read_string_and_trim(args, file_name)) {
                cli_print_usage(
                    "subghz decode_raw", "<file_name: path_RAW_file>", furi_string_get_cstr(args));
                break;
            }
        }

        if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
            printf(
                "subghz decode_raw \033[0;31mError open file\033[0m %s\r\n",
                furi_string_get_cstr(file_name));
            break;
        }

        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            printf("subghz decode_raw \033[0;31mMissing or incorrect header\033[0m\r\n");
            break;
        }

        if(!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            printf("subghz decode_raw \033[0;31mType or version mismatch\033[0m\r\n");
            break;
        }

        check_file = true;
    } while(false);

    furi_string_free(temp_str);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    if(check_file) {
        // Allocate context
        SubGhzCliCommandRx* instance = malloc(sizeof(SubGhzCliCommandRx));

        SubGhzEnvironment* environment = subghz_environment_alloc();
        if(subghz_environment_load_keystore(
               environment, EXT_PATH("subghz/assets/keeloq_mfcodes"))) {
            printf("SubGhz decode_raw: Load_keystore keeloq_mfcodes \033[0;32mOK\033[0m\r\n");
        } else {
            printf("SubGhz decode_raw: Load_keystore keeloq_mfcodes \033[0;31mERROR\033[0m\r\n");
        }
        if(subghz_environment_load_keystore(
               environment, EXT_PATH("subghz/assets/keeloq_mfcodes_user"))) {
            printf("SubGhz decode_raw: Load_keystore keeloq_mfcodes_user \033[0;32mOK\033[0m\r\n");
        } else {
            printf(
                "SubGhz decode_raw: Load_keystore keeloq_mfcodes_user \033[0;31mERROR\033[0m\r\n");
        }
        subghz_environment_set_came_atomo_rainbow_table_file_name(
            environment, EXT_PATH("subghz/assets/came_atomo"));
        subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
            environment, EXT_PATH("subghz/assets/alutech_at_4n"));
        subghz_environment_set_nice_flor_s_rainbow_table_file_name(
            environment, EXT_PATH("subghz/assets/nice_flor_s"));
        subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);

        SubGhzReceiver* receiver = subghz_receiver_alloc_init(environment);
        subghz_receiver_set_filter(receiver, SubGhzProtocolFlag_Decodable);
        subghz_receiver_set_rx_callback(receiver, subghz_cli_command_rx_callback, instance);

        SubGhzFileEncoderWorker* file_worker_encoder = subghz_file_encoder_worker_alloc();
        if(subghz_file_encoder_worker_start(file_worker_encoder, furi_string_get_cstr(file_name))) {
            //the worker needs a file in order to open and read part of the file
            furi_delay_ms(100);
        }

        printf(
            "Listening at \033[0;33m%s\033[0m.\r\n\r\nPress CTRL+C to stop\r\n\r\n",
            furi_string_get_cstr(file_name));

        LevelDuration level_duration;
        while(!cli_cmd_interrupt_received(cli)) {
            furi_delay_us(500); //you need to have time to read from the file from the SD card
            level_duration = subghz_file_encoder_worker_get_level_duration(file_worker_encoder);
            if(!level_duration_is_reset(level_duration)) {
                bool level = level_duration_get_level(level_duration);
                uint32_t duration = level_duration_get_duration(level_duration);
                subghz_receiver_decode(receiver, level, duration);
            } else {
                break;
            }
        }

        printf("\r\nPackets received \033[0;32m%u\033[0m\r\n", instance->packet_count);

        // Cleanup
        subghz_receiver_free(receiver);
        subghz_environment_free(environment);

        if(subghz_file_encoder_worker_is_running(file_worker_encoder)) {
            subghz_file_encoder_worker_stop(file_worker_encoder);
        }
        subghz_file_encoder_worker_free(file_worker_encoder);
        free(instance);
    }
    furi_string_free(file_name);
}

static void subghz_cli_command_print_usage() {
    printf("Usage:\r\n");
    printf("subghz <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf("\tchat <frequency:in Hz>\t - Chat with other Flippers\r\n");
    printf(
        "\ttx <3 byte Key: in hex> <frequency: in Hz> <te: us> <repeat: count>\t - Transmitting key\r\n");
    printf("\trx <frequency:in Hz>\t - Receive\r\n");
    printf("\trx_raw <frequency:in Hz>\t - Receive RAW\r\n");
    printf("\tdecode_raw <file_name: path_RAW_file>\t - Testing\r\n");

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        printf("\r\n");
        printf("  debug cmd:\r\n");
        printf("\ttx_carrier <frequency:in Hz>\t - Transmit carrier\r\n");
        printf("\trx_carrier <frequency:in Hz>\t - Receive carrier\r\n");
        printf(
            "\tencrypt_keeloq <path_decrypted_file> <path_encrypted_file> <IV:16 bytes in hex>\t - Encrypt keeloq manufacture keys\r\n");
        printf(
            "\tencrypt_raw <path_decrypted_file> <path_encrypted_file> <IV:16 bytes in hex>\t - Encrypt RAW data\r\n");
    }
}

static void subghz_cli_command_encrypt_keeloq(Cli* cli, FuriString* args) {
    UNUSED(cli);
    uint8_t iv[16];

    FuriString* source;
    FuriString* destination;
    source = furi_string_alloc();
    destination = furi_string_alloc();

    SubGhzKeystore* keystore = subghz_keystore_alloc();

    do {
        if(!args_read_string_and_trim(args, source)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(!args_read_string_and_trim(args, destination)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(!args_read_hex_bytes(args, iv, 16)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(!subghz_keystore_load(keystore, furi_string_get_cstr(source))) {
            printf("Failed to load Keystore");
            break;
        }

        if(!subghz_keystore_save(keystore, furi_string_get_cstr(destination), iv)) {
            printf("Failed to save Keystore");
            break;
        }
    } while(false);

    subghz_keystore_free(keystore);
    furi_string_free(destination);
    furi_string_free(source);
}

static void subghz_cli_command_encrypt_raw(Cli* cli, FuriString* args) {
    UNUSED(cli);
    uint8_t iv[16];

    FuriString* source;
    FuriString* destination;
    source = furi_string_alloc();
    destination = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, source)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(!args_read_string_and_trim(args, destination)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(!args_read_hex_bytes(args, iv, 16)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(!subghz_keystore_raw_encrypted_save(
               furi_string_get_cstr(source), furi_string_get_cstr(destination), iv)) {
            printf("Failed to save Keystore");
            break;
        }

    } while(false);

    furi_string_free(destination);
    furi_string_free(source);
}

static void subghz_cli_command_chat(Cli* cli, FuriString* args) {
    uint32_t frequency = 433920000;

    if(furi_string_size(args)) {
        int ret = sscanf(furi_string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz chat", "<Frequency: in Hz>", furi_string_get_cstr(args));
            return;
        }
        if(!furi_hal_subghz_is_frequency_valid(frequency)) {
            printf(
                "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }
    if(!furi_hal_region_is_frequency_allowed(frequency)) {
        printf(
            "In your region, only reception on this frequency (%lu) is allowed,\r\n"
            "the actual operation of the application is not possible\r\n ",
            frequency);
        return;
    }

    SubGhzChatWorker* subghz_chat = subghz_chat_worker_alloc(cli);
    if(!subghz_chat_worker_start(subghz_chat, frequency)) {
        printf("Startup error SubGhzChatWorker\r\n");

        if(subghz_chat_worker_is_running(subghz_chat)) {
            subghz_chat_worker_stop(subghz_chat);
            subghz_chat_worker_free(subghz_chat);
        }
        return;
    }

    printf("Receiving at frequency %lu Hz\r\n", frequency);
    printf("Press CTRL+C to stop\r\n");

    furi_hal_power_suppress_charge_enter();

    size_t message_max_len = 64;
    uint8_t message[64] = {0};
    FuriString* input;
    input = furi_string_alloc();
    FuriString* name;
    name = furi_string_alloc();
    FuriString* output;
    output = furi_string_alloc();
    FuriString* sysmsg;
    sysmsg = furi_string_alloc();
    bool exit = false;
    SubGhzChatEvent chat_event;

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);

    furi_string_printf(name, "\033[0;33m%s\033[0m: ", furi_hal_version_get_name_ptr());
    furi_string_set(input, name);
    printf("%s", furi_string_get_cstr(input));
    fflush(stdout);

    while(!exit) {
        chat_event = subghz_chat_worker_get_event_chat(subghz_chat);
        switch(chat_event.event) {
        case SubGhzChatEventInputData:
            if(chat_event.c == CliSymbolAsciiETX) {
                printf("\r\n");
                chat_event.event = SubGhzChatEventUserExit;
                subghz_chat_worker_put_event_chat(subghz_chat, &chat_event);
                break;
            } else if(
                (chat_event.c == CliSymbolAsciiBackspace) || (chat_event.c == CliSymbolAsciiDel)) {
                size_t len = furi_string_utf8_length(input);
                if(len > furi_string_utf8_length(name)) {
                    printf("%s", "\e[D\e[1P");
                    fflush(stdout);
                    //delete 1 char UTF
                    const char* str = furi_string_get_cstr(input);
                    size_t size = 0;
                    FuriStringUTF8State s = FuriStringUTF8StateStarting;
                    FuriStringUnicodeValue u = 0;
                    furi_string_reset(sysmsg);
                    while(*str) {
                        furi_string_utf8_decode(*str, &s, &u);
                        if((s == FuriStringUTF8StateError) || s == FuriStringUTF8StateStarting) {
                            furi_string_utf8_push(sysmsg, u);
                            if(++size >= len - 1) break;
                            s = FuriStringUTF8StateStarting;
                        }
                        str++;
                    }
                    furi_string_set(input, sysmsg);
                }
            } else if(chat_event.c == CliSymbolAsciiCR) {
                printf("\r\n");
                furi_string_push_back(input, '\r');
                furi_string_push_back(input, '\n');
                while(!subghz_chat_worker_write(
                    subghz_chat,
                    (uint8_t*)furi_string_get_cstr(input),
                    strlen(furi_string_get_cstr(input)))) {
                    furi_delay_ms(10);
                }

                furi_string_printf(input, "%s", furi_string_get_cstr(name));
                printf("%s", furi_string_get_cstr(input));
                fflush(stdout);
            } else if(chat_event.c == CliSymbolAsciiLF) {
                //cut out the symbol \n
            } else {
                putc(chat_event.c, stdout);
                fflush(stdout);
                furi_string_push_back(input, chat_event.c);
                break;
            case SubGhzChatEventRXData:
                do {
                    memset(message, 0x00, message_max_len);
                    size_t len = subghz_chat_worker_read(subghz_chat, message, message_max_len);
                    for(size_t i = 0; i < len; i++) {
                        furi_string_push_back(output, message[i]);
                        if(message[i] == '\n') {
                            printf("\r");
                            for(uint8_t i = 0; i < 80; i++) {
                                printf(" ");
                            }
                            printf("\r %s", furi_string_get_cstr(output));
                            printf("%s", furi_string_get_cstr(input));
                            fflush(stdout);
                            furi_string_reset(output);
                        }
                    }
                } while(subghz_chat_worker_available(subghz_chat));
                break;
            case SubGhzChatEventNewMessage:
                notification_message(notification, &sequence_single_vibro);
                break;
            case SubGhzChatEventUserEntrance:
                furi_string_printf(
                    sysmsg,
                    "\033[0;34m%s joined chat.\033[0m\r\n",
                    furi_hal_version_get_name_ptr());
                subghz_chat_worker_write(
                    subghz_chat,
                    (uint8_t*)furi_string_get_cstr(sysmsg),
                    strlen(furi_string_get_cstr(sysmsg)));
                break;
            case SubGhzChatEventUserExit:
                furi_string_printf(
                    sysmsg, "\033[0;31m%s left chat.\033[0m\r\n", furi_hal_version_get_name_ptr());
                subghz_chat_worker_write(
                    subghz_chat,
                    (uint8_t*)furi_string_get_cstr(sysmsg),
                    strlen(furi_string_get_cstr(sysmsg)));
                furi_delay_ms(10);
                exit = true;
                break;
            default:
                FURI_LOG_W("SubGhzChat", "Error event");
                break;
            }
        }
        if(!cli_is_connected(cli)) {
            printf("\r\n");
            chat_event.event = SubGhzChatEventUserExit;
            subghz_chat_worker_put_event_chat(subghz_chat, &chat_event);
        }
    }

    furi_string_free(input);
    furi_string_free(name);
    furi_string_free(output);
    furi_string_free(sysmsg);
    furi_hal_power_suppress_charge_exit();
    furi_record_close(RECORD_NOTIFICATION);

    if(subghz_chat_worker_is_running(subghz_chat)) {
        subghz_chat_worker_stop(subghz_chat);
        subghz_chat_worker_free(subghz_chat);
    }
    printf("\r\nExit chat\r\n");
}

static void subghz_cli_command(Cli* cli, FuriString* args, void* context) {
    FuriString* cmd;
    cmd = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(furi_string_cmp_str(cmd, "chat") == 0) {
            subghz_cli_command_chat(cli, args);
            break;
        }

        if(furi_string_cmp_str(cmd, "tx") == 0) {
            subghz_cli_command_tx(cli, args, context);
            break;
        }

        if(furi_string_cmp_str(cmd, "rx") == 0) {
            subghz_cli_command_rx(cli, args, context);
            break;
        }

        if(furi_string_cmp_str(cmd, "rx_raw") == 0) {
            subghz_cli_command_rx_raw(cli, args, context);
            break;
        }

        if(furi_string_cmp_str(cmd, "decode_raw") == 0) {
            subghz_cli_command_decode_raw(cli, args, context);
            break;
        }

        if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
            if(furi_string_cmp_str(cmd, "encrypt_keeloq") == 0) {
                subghz_cli_command_encrypt_keeloq(cli, args);
                break;
            }

            if(furi_string_cmp_str(cmd, "encrypt_raw") == 0) {
                subghz_cli_command_encrypt_raw(cli, args);
                break;
            }

            if(furi_string_cmp_str(cmd, "tx_carrier") == 0) {
                subghz_cli_command_tx_carrier(cli, args, context);
                break;
            }

            if(furi_string_cmp_str(cmd, "rx_carrier") == 0) {
                subghz_cli_command_rx_carrier(cli, args, context);
                break;
            }
        }

        subghz_cli_command_print_usage();
    } while(false);

    furi_string_free(cmd);
}

static bool
    subghz_on_system_start_istream_read(pb_istream_t* istream, pb_byte_t* buf, size_t count) {
    File* file = istream->state;
    uint16_t ret = storage_file_read(file, buf, count);
    return (count == ret);
}

static bool subghz_on_system_start_istream_decode_band(
    pb_istream_t* stream,
    const pb_field_t* field,
    void** arg) {
    (void)field;
    FuriHalRegion* region = *arg;

    PB_Region_Band band = {0};
    if(!pb_decode(stream, PB_Region_Band_fields, &band)) {
        FURI_LOG_E("SubGhzOnStart", "PB Region band decode error: %s", PB_GET_ERROR(stream));
        return false;
    }

    region->bands_count += 1;
    region = realloc( //-V701
        region,
        sizeof(FuriHalRegion) + sizeof(FuriHalRegionBand) * region->bands_count);
    size_t pos = region->bands_count - 1;
    region->bands[pos].start = band.start;
    region->bands[pos].end = band.end;
    region->bands[pos].power_limit = band.power_limit;
    region->bands[pos].duty_cycle = band.duty_cycle;
    *arg = region;

    FURI_LOG_I(
        "SubGhzOnStart",
        "Add allowed band: start %luHz, stop %luHz, power_limit %ddBm, duty_cycle %u%%",
        band.start,
        band.end,
        band.power_limit,
        band.duty_cycle);
    return true;
}

void subghz_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "subghz", CliCommandFlagDefault, subghz_cli_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(subghz_cli_command);
#endif

#ifdef SRV_STORAGE
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    FileInfo fileinfo = {0};
    PB_Region pb_region = {0};
    pb_region.bands.funcs.decode = subghz_on_system_start_istream_decode_band;

    do {
        if(storage_common_stat(storage, SUBGHZ_REGION_FILENAME, &fileinfo) != FSE_OK ||
           fileinfo.size == 0) {
            FURI_LOG_W("SubGhzOnStart", "Region data is missing or empty");
            break;
        }

        if(!storage_file_open(file, SUBGHZ_REGION_FILENAME, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E("SubGhzOnStart", "Unable to open region data");
            break;
        }

        pb_istream_t istream = {
            .callback = subghz_on_system_start_istream_read,
            .state = file,
            .errmsg = NULL,
            .bytes_left = fileinfo.size,
        };

        pb_region.bands.arg = malloc(sizeof(FuriHalRegion));
        if(!pb_decode(&istream, PB_Region_fields, &pb_region)) {
            FURI_LOG_E("SubGhzOnStart", "Invalid region data");
            free(pb_region.bands.arg);
            break;
        }

        FuriHalRegion* region = pb_region.bands.arg;
        memcpy(
            region->country_code,
            pb_region.country_code->bytes,
            pb_region.country_code->size < 4 ? pb_region.country_code->size : 3);
        furi_hal_region_set(region);
    } while(0);

    pb_release(PB_Region_fields, &pb_region);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
#else
    UNUSED(subghz_cli_command);
    UNUSED(subghz_on_system_start_istream_decode_band);
    UNUSED(subghz_on_system_start_istream_read);
#endif
}

#include "subghz_cli.h"

#include <furi.h>
#include <furi-hal.h>
#include <stream_buffer.h>

#include <lib/toolbox/args.h>
#include <lib/subghz/subghz_parser.h>
#include <lib/subghz/subghz_keystore.h>
#include <lib/subghz/protocols/subghz_protocol_common.h>
#include <lib/subghz/protocols/subghz_protocol_princeton.h>

#define SUBGHZ_FREQUENCY_RANGE_STR \
    "299999755...348000000 or 386999938...464000000 or 778999847...928000000"

void subghz_cli_command_tx_carrier(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 433920000;

    if(string_size(args)) {
        int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz_tx_carrier", "<Frequency in HZ>", string_get_cstr(args));
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

    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, true);

    furi_hal_power_suppress_charge_enter();

    if(furi_hal_subghz_tx()) {
        printf("Transmitting at frequency %lu Hz\r\n", frequency);
        printf("Press CTRL+C to stop\r\n");
        while(!cli_cmd_interrupt_received(cli)) {
            osDelay(250);
        }
    } else {
        printf("This frequency can only be used for RX in your region\r\n");
    }

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();
}

void subghz_cli_command_rx_carrier(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 433920000;

    if(string_size(args)) {
        int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz_tx_carrier", "<Frequency in HZ>", string_get_cstr(args));
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
        osDelay(250);
        printf("RSSI: %03.1fdbm\r", furi_hal_subghz_get_rssi());
        fflush(stdout);
    }

    furi_hal_power_suppress_charge_exit();

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
}

void subghz_cli_command_tx(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 433920000;
    uint32_t key = 0x0074BADE;
    size_t repeat = 10;

    if(string_size(args)) {
        int ret = sscanf(string_get_cstr(args), "%lx %lu %u", &key, &frequency, &repeat);
        if(ret != 3) {
            printf(
                "sscanf returned %d, key: %lx, frequency: %lu, repeat: %u\r\n",
                ret,
                key,
                frequency,
                repeat);
            cli_print_usage(
                "subghz_rx",
                "<3 Byte Key in hex> <Frequency in HZ> <Repeat count>",
                string_get_cstr(args));
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
        "Transmitting at %lu, key %lx, repeat %u. Press CTRL+C to stop\r\n",
        frequency,
        key,
        repeat);

    SubGhzDecoderPrinceton* protocol = subghz_decoder_princeton_alloc();
    protocol->common.code_last_found = key;
    protocol->common.code_last_count_bit = 24;

    SubGhzProtocolCommonEncoder* encoder = subghz_protocol_encoder_common_alloc();
    encoder->repeat = repeat;

    subghz_protocol_princeton_send_key(protocol, encoder);
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    frequency = furi_hal_subghz_set_frequency_and_path(frequency);

    furi_hal_power_suppress_charge_enter();

    furi_hal_subghz_start_async_tx(subghz_protocol_encoder_common_yield, encoder);

    while(!(furi_hal_subghz_is_async_tx_complete() || cli_cmd_interrupt_received(cli))) {
        printf(".");
        fflush(stdout);
        osDelay(333);
    }
    furi_hal_subghz_stop_async_tx();
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();

    subghz_decoder_princeton_free(protocol);
    subghz_protocol_encoder_common_free(encoder);
}

typedef struct {
    volatile bool overrun;
    StreamBufferHandle_t stream;
    size_t packet_count;
} SubGhzCliCommandRx;

static void subghz_cli_command_rx_callback(bool level, uint32_t duration, void* context) {
    SubGhzCliCommandRx* instance = context;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    LevelDuration level_duration = level_duration_make(level, duration);
    if(instance->overrun) {
        instance->overrun = false;
        level_duration = level_duration_reset();
    }
    size_t ret = xStreamBufferSendFromISR(
        instance->stream, &level_duration, sizeof(LevelDuration), &xHigherPriorityTaskWoken);
    if(sizeof(LevelDuration) != ret) instance->overrun = true;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void subghz_cli_command_rx_text_callback(string_t text, void* context) {
    SubGhzCliCommandRx* instance = context;
    instance->packet_count++;
    printf("%s", string_get_cstr(text));
}

void subghz_cli_command_rx(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 433920000;

    if(string_size(args)) {
        int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz_rx", "<Frequency in HZ>", string_get_cstr(args));
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
    SubGhzCliCommandRx* instance = furi_alloc(sizeof(SubGhzCliCommandRx));
    instance->stream = xStreamBufferCreate(sizeof(LevelDuration) * 1024, sizeof(LevelDuration));
    furi_check(instance->stream);

    SubGhzParser* parser = subghz_parser_alloc();
    subghz_parser_load_keeloq_file(parser, "/ext/subghz/keeloq_mfcodes");
    subghz_parser_load_keeloq_file(parser, "/ext/subghz/keeloq_mfcodes_user");
    subghz_parser_load_nice_flor_s_file(parser, "/ext/subghz/nice_flor_s_rx");
    subghz_parser_load_came_atomo_file(parser, "/ext/subghz/came_atomo");
    subghz_parser_enable_dump_text(parser, subghz_cli_command_rx_text_callback, instance);

    // Configure radio
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(FuriHalSubGhzPresetOok650Async);
    frequency = furi_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    furi_hal_power_suppress_charge_enter();

    // Prepare and start RX
    furi_hal_subghz_start_async_rx(subghz_cli_command_rx_callback, instance);

    // Wait for packets to arrive
    printf("Listening at %lu. Press CTRL+C to stop\r\n", frequency);
    LevelDuration level_duration;
    while(!cli_cmd_interrupt_received(cli)) {
        int ret =
            xStreamBufferReceive(instance->stream, &level_duration, sizeof(LevelDuration), 10);
        if(ret == sizeof(LevelDuration)) {
            if(level_duration_is_reset(level_duration)) {
                printf(".");
                subghz_parser_reset(parser);
            } else {
                bool level = level_duration_get_level(level_duration);
                uint32_t duration = level_duration_get_duration(level_duration);
                subghz_parser_parse(parser, level, duration);
            }
        }
    }

    // Shutdown radio
    furi_hal_subghz_stop_async_rx();
    furi_hal_subghz_sleep();

    furi_hal_power_suppress_charge_exit();

    printf("\r\nPackets recieved %u\r\n", instance->packet_count);

    // Cleanup
    subghz_parser_free(parser);
    vStreamBufferDelete(instance->stream);
    free(instance);
}

void subghz_cli_command_print_usage() {
    printf("Usage:\r\n");
    printf("subghz <cmd> <args>\r\n");
    printf("Cmd list:\r\n");
    printf(
        "\tencrypt_keeloq <path_decrypted_file> <path_encrypted_file> <IV:16 bytes in hex>\t - Encrypt keeloq manufacture keys\r\n");
    printf(
        "\tencrypt_raw <path_decrypted_file> <path_encrypted_file> <IV:16 bytes in hex>\t - Encrypt RAW data\r\n");
}

void subghz_cli_command_encrypt_keeloq(Cli* cli, string_t args) {
    uint8_t iv[16];

    string_t source;
    string_t destination;
    string_init(source);
    string_init(destination);

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

        if(!subghz_keystore_load(keystore, string_get_cstr(source))) {
            printf("Failed to load Keystore");
            break;
        }

        if(!subghz_keystore_save(keystore, string_get_cstr(destination), iv)) {
            printf("Failed to save Keystore");
            break;
        }
    } while(false);

    subghz_keystore_free(keystore);
    string_clear(destination);
    string_clear(source);
}

void subghz_cli_command_encrypt_raw(Cli* cli, string_t args) {
    uint8_t iv[16];

    string_t source;
    string_t destination;
    string_init(source);
    string_init(destination);

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
               string_get_cstr(source), string_get_cstr(destination), iv)) {
            printf("Failed to save Keystore");
            break;
        }

    } while(false);

    string_clear(destination);
    string_clear(source);
}

void subghz_cli_command(Cli* cli, string_t args, void* context) {
    string_t cmd;
    string_init(cmd);

    do {
        if(!args_read_string_and_trim(args, cmd)) {
            subghz_cli_command_print_usage();
            break;
        }

        if(string_cmp_str(cmd, "encrypt_keeloq") == 0) {
            subghz_cli_command_encrypt_keeloq(cli, args);
            break;
        }

        if(string_cmp_str(cmd, "encrypt_raw") == 0) {
            subghz_cli_command_encrypt_raw(cli, args);
            break;
        }

        subghz_cli_command_print_usage();
    } while(false);

    string_clear(cmd);
}

void subghz_cli_init() {
    Cli* cli = furi_record_open("cli");

    cli_add_command(
        cli, "subghz_tx_carrier", CliCommandFlagDefault, subghz_cli_command_tx_carrier, NULL);
    cli_add_command(
        cli, "subghz_rx_carrier", CliCommandFlagDefault, subghz_cli_command_rx_carrier, NULL);
    cli_add_command(cli, "subghz_tx", CliCommandFlagDefault, subghz_cli_command_tx, NULL);
    cli_add_command(cli, "subghz_rx", CliCommandFlagDefault, subghz_cli_command_rx, NULL);
    cli_add_command(cli, "subghz", CliCommandFlagDefault, subghz_cli_command, NULL);

    furi_record_close("cli");
}

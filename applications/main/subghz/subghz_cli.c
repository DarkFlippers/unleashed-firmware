#include "subghz_cli.h"

#include <furi.h>
#include <furi_hal.h>

#include <applications/drivers/subghz/cc1101_ext/cc1101_ext_interconnect.h>

#include <lib/subghz/subghz_keystore.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/subghz_file_encoder_worker.h>
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/devices/cc1101_configs.h>

#include <lib/toolbox/args.h>
#include <lib/toolbox/strint.h>

#include "helpers/subghz_chat.h"

#include <notification/notification_messages.h>
#include <flipper_format/flipper_format_i.h>

#define SUBGHZ_FREQUENCY_RANGE_STR \
    "299999755...348000000 or 386999938...464000000 or 778999847...928000000"

#define TAG "SubGhzCli"

static void subghz_cli_radio_device_power_on(void) {
    uint8_t attempts = 5;
    while(--attempts > 0) {
        if(furi_hal_power_enable_otg()) break;
    }
    if(attempts == 0) {
        if(furi_hal_power_get_usb_voltage() < 4.5f) {
            FURI_LOG_E(
                "TAG",
                "Error power otg enable. BQ2589 check otg fault = %d",
                furi_hal_power_check_otg_fault() ? 1 : 0);
        }
    }
}

static void subghz_cli_radio_device_power_off(void) {
    if(furi_hal_power_is_otg_enabled()) furi_hal_power_disable_otg();
}

static SubGhzEnvironment* subghz_cli_environment_init(void) {
    SubGhzEnvironment* environment = subghz_environment_alloc();
    if(subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_NAME)) {
        printf("Load_keystore keeloq_mfcodes \033[0;32mOK\033[0m\r\n");
    } else {
        printf("Load_keystore keeloq_mfcodes \033[0;31mERROR\033[0m\r\n");
    }
    if(subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_USER_NAME)) {
        printf("Load_keystore keeloq_mfcodes_user \033[0;32mOK\033[0m\r\n");
    } else {
        printf("Load_keystore keeloq_mfcodes_user \033[0;33mAbsent\033[0m\r\n");
    }
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        environment, SUBGHZ_CAME_ATOMO_DIR_NAME);
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        environment, SUBGHZ_ALUTECH_AT_4N_DIR_NAME);
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        environment, SUBGHZ_NICE_FLOR_S_DIR_NAME);
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);
    return environment;
}

void subghz_cli_command_tx_carrier(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    uint32_t frequency = 433920000;

    if(furi_string_size(args)) {
        if(strint_to_uint32(furi_string_get_cstr(args), NULL, &frequency, 10) !=
           StrintParseNoError) {
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
    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);
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
        if(strint_to_uint32(furi_string_get_cstr(args), NULL, &frequency, 10) !=
           StrintParseNoError) {
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
    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_650khz_async_regs);
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

static const SubGhzDevice* subghz_cli_command_get_device(uint32_t* device_ind) {
    const SubGhzDevice* device = NULL;
    switch(*device_ind) {
    case 1:
        subghz_cli_radio_device_power_on();
        device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_EXT_NAME);
        break;

    default:
        device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
        break;
    }
    //check if the device is connected
    if(!subghz_devices_is_connect(device)) {
        subghz_cli_radio_device_power_off();
        device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
        *device_ind = 0;
    }
    return device;
}

void subghz_cli_command_tx(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    uint32_t frequency = 433920000;
    uint32_t key = 0x0074BADE;
    uint32_t repeat = 10;
    uint32_t te = 403;
    uint32_t device_ind = 0; // 0 - CC1101_INT, 1 - CC1101_EXT

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &key, 16);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &frequency, 10);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &te, 10);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &repeat, 10);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &device_ind, 10);
        if(parse_err) {
            cli_print_usage(
                "subghz tx",
                "<3 Byte Key: in hex> <Frequency: in Hz> <Te us> <Repeat count> <Device: 0 - CC1101_INT, 1 - CC1101_EXT>",
                furi_string_get_cstr(args));
            return;
        }
    }
    subghz_devices_init();
    const SubGhzDevice* device = subghz_cli_command_get_device(&device_ind);
    if(!subghz_devices_is_frequency_valid(device, frequency)) {
        printf(
            "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n", frequency);
        subghz_devices_deinit();
        subghz_cli_radio_device_power_off();
        return;
    }
    printf(
        "Transmitting at %lu, key %lx, te %lu, repeat %lu device %lu. Press CTRL+C to stop\r\n",
        frequency,
        key,
        te,
        repeat,
        device_ind);

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

    subghz_devices_begin(device);
    subghz_devices_reset(device);
    subghz_devices_load_preset(device, FuriHalSubGhzPresetOok650Async, NULL);
    frequency = subghz_devices_set_frequency(device, frequency);

    furi_hal_power_suppress_charge_enter();
    if(subghz_devices_start_async_tx(device, subghz_transmitter_yield, transmitter)) {
        while(!(subghz_devices_is_async_complete_tx(device) || cli_cmd_interrupt_received(cli))) {
            printf(".");
            fflush(stdout);
            furi_delay_ms(333);
        }
        subghz_devices_stop_async_tx(device);

    } else {
        printf("Transmission on this frequency is restricted in your region\r\n");
    }

    subghz_devices_sleep(device);
    subghz_devices_end(device);
    subghz_devices_deinit();
    subghz_cli_radio_device_power_off();

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
    uint32_t device_ind = 0; // 0 - CC1101_INT, 1 - CC1101_EXT

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &frequency, 10);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &device_ind, 10);
        if(parse_err) {
            cli_print_usage(
                "subghz rx",
                "<Frequency: in Hz> <Device: 0 - CC1101_INT, 1 - CC1101_EXT>",
                furi_string_get_cstr(args));
            return;
        }
    }
    subghz_devices_init();
    const SubGhzDevice* device = subghz_cli_command_get_device(&device_ind);
    if(!subghz_devices_is_frequency_valid(device, frequency)) {
        printf(
            "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n", frequency);
        subghz_devices_deinit();
        subghz_cli_radio_device_power_off();
        return;
    }

    // Allocate context and buffers
    SubGhzCliCommandRx* instance = malloc(sizeof(SubGhzCliCommandRx));
    instance->stream =
        furi_stream_buffer_alloc(sizeof(LevelDuration) * 1024, sizeof(LevelDuration));

    SubGhzEnvironment* environment = subghz_cli_environment_init();

    SubGhzReceiver* receiver = subghz_receiver_alloc_init(environment);
    subghz_receiver_set_filter(receiver, SubGhzProtocolFlag_Decodable);
    subghz_receiver_set_rx_callback(receiver, subghz_cli_command_rx_callback, instance);

    // Configure radio
    subghz_devices_begin(device);
    subghz_devices_reset(device);
    subghz_devices_load_preset(device, FuriHalSubGhzPresetOok650Async, NULL);
    frequency = subghz_devices_set_frequency(device, frequency);

    furi_hal_power_suppress_charge_enter();

    // Prepare and start RX
    subghz_devices_start_async_rx(device, subghz_cli_command_rx_capture_callback, instance);

    // Wait for packets to arrive
    printf(
        "Listening at frequency: %lu device: %lu. Press CTRL+C to stop\r\n",
        frequency,
        device_ind);
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
    subghz_devices_stop_async_rx(device);
    subghz_devices_sleep(device);
    subghz_devices_end(device);
    subghz_devices_deinit();
    subghz_cli_radio_device_power_off();

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
        if(strint_to_uint32(furi_string_get_cstr(args), NULL, &frequency, 10) !=
           StrintParseNoError) {
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

    // Configure radio
    furi_hal_subghz_reset();
    furi_hal_subghz_load_custom_preset(subghz_device_cc1101_preset_ook_270khz_async_regs);
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
    furi_string_set(file_name, EXT_PATH("subghz/test.sub"));

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

        SubGhzEnvironment* environment = subghz_cli_environment_init();

        SubGhzReceiver* receiver = subghz_receiver_alloc_init(environment);
        subghz_receiver_set_filter(receiver, SubGhzProtocolFlag_Decodable);
        subghz_receiver_set_rx_callback(receiver, subghz_cli_command_rx_callback, instance);

        SubGhzFileEncoderWorker* file_worker_encoder = subghz_file_encoder_worker_alloc();
        if(subghz_file_encoder_worker_start(
               file_worker_encoder, furi_string_get_cstr(file_name), NULL)) {
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

        printf("\r\nPackets received \033[0;32m%zu\033[0m\r\n", instance->packet_count);

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

static FuriHalSubGhzPreset subghz_cli_get_preset_name(const char* preset_name) {
    FuriHalSubGhzPreset preset = FuriHalSubGhzPresetIDLE;
    if(!strcmp(preset_name, "FuriHalSubGhzPresetOok270Async")) {
        preset = FuriHalSubGhzPresetOok270Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPresetOok650Async")) {
        preset = FuriHalSubGhzPresetOok650Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPreset2FSKDev238Async")) {
        preset = FuriHalSubGhzPreset2FSKDev238Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPreset2FSKDev476Async")) {
        preset = FuriHalSubGhzPreset2FSKDev476Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPresetCustom")) {
        preset = FuriHalSubGhzPresetCustom;
    } else {
        printf("subghz tx_from_file: unknown preset");
    }
    return preset;
}

void subghz_cli_command_tx_from_file(Cli* cli, FuriString* args, void* context) { // -V524
    UNUSED(context);
    FuriString* file_name;
    file_name = furi_string_alloc();
    furi_string_set(file_name, EXT_PATH("subghz/test.sub"));
    uint32_t repeat = 10;
    uint32_t device_ind = 0; // 0 - CC1101_INT, 1 - CC1101_EXT

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    FlipperFormat* fff_data_raw = flipper_format_string_alloc();
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    uint32_t temp_data32;
    bool check_file = false;
    const SubGhzDevice* device = NULL;

    uint32_t frequency = 0;
    SubGhzTransmitter* transmitter = NULL;

    subghz_devices_init();

    SubGhzEnvironment* environment = subghz_cli_environment_init();

    do {
        if(furi_string_size(args)) {
            if(!args_read_string_and_trim(args, file_name)) {
                cli_print_usage(
                    "subghz tx_from_file: ",
                    "<file_name: path_file> <Repeat count> <Device: 0 - CC1101_INT, 1 - CC1101_EXT>",
                    furi_string_get_cstr(args));
                break;
            }
        }

        if(furi_string_size(args)) {
            char* args_cstr = (char*)furi_string_get_cstr(args);
            StrintParseError parse_err = StrintParseNoError;
            parse_err |= strint_to_uint32(args_cstr, &args_cstr, &frequency, 10);
            parse_err |= strint_to_uint32(args_cstr, &args_cstr, &device_ind, 10);
            if(parse_err) {
                cli_print_usage(
                    "subghz tx_from_file:",
                    "<file_name: path_file> <Repeat count> <Device: 0 - CC1101_INT, 1 - CC1101_EXT>",
                    furi_string_get_cstr(args));
                break;
            }
        }

        device = subghz_cli_command_get_device(&device_ind);
        if(device == NULL) {
            printf("subghz tx_from_file: \033[0;31mError device not found\033[0m\r\n");
            break;
        }

        if(!flipper_format_file_open_existing(fff_data_file, furi_string_get_cstr(file_name))) {
            printf(
                "subghz tx_from_file: \033[0;31mError open file\033[0m %s\r\n",
                furi_string_get_cstr(file_name));
            break;
        }

        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            printf("subghz tx_from_file: \033[0;31mMissing or incorrect header\033[0m\r\n");
            break;
        }

        if(((!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            printf("subghz tx_from_file: \033[0;31mType or version mismatch\033[0m\r\n");
            break;
        }

        //Load frequency
        if(!flipper_format_read_uint32(fff_data_file, "Frequency", &frequency, 1)) {
            printf("subghz tx_from_file: \033[0;31mMissing Frequency\033[0m\r\n");
            break;
        }

        if(!subghz_devices_is_frequency_valid(device, frequency)) {
            printf("subghz tx_from_file: \033[0;31mFrequency not supported\033[0m\r\n");
            break;
        }

        //Load preset
        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            printf("subghz tx_from_file: \033[0;31mMissing Preset\033[0m\r\n");
            break;
        }

        subghz_devices_begin(device);
        subghz_devices_reset(device);

        if(!strcmp(furi_string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
            uint8_t* custom_preset_data;
            uint32_t custom_preset_data_size;
            if(!flipper_format_get_value_count(fff_data_file, "Custom_preset_data", &temp_data32))
                break;
            if(!temp_data32 || (temp_data32 % 2)) {
                printf("subghz tx_from_file: \033[0;31mCustom_preset_data size error\033[0m\r\n");
                break;
            }
            custom_preset_data_size = sizeof(uint8_t) * temp_data32;
            custom_preset_data = malloc(custom_preset_data_size);
            if(!flipper_format_read_hex(
                   fff_data_file,
                   "Custom_preset_data",
                   custom_preset_data,
                   custom_preset_data_size)) {
                printf("subghz tx_from_file: \033[0;31mCustom_preset_data read error\033[0m\r\n");
                break;
            }
            subghz_devices_load_preset(
                device,
                subghz_cli_get_preset_name(furi_string_get_cstr(temp_str)),
                custom_preset_data);
            free(custom_preset_data);
        } else {
            subghz_devices_load_preset(
                device, subghz_cli_get_preset_name(furi_string_get_cstr(temp_str)), NULL);
        }

        subghz_devices_set_frequency(device, frequency);

        //Load protocol
        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            printf("subghz tx_from_file: \033[0;31mMissing protocol\033[0m\r\n");
            break;
        }

        SubGhzProtocolStatus status;
        bool is_init_protocol = true;
        if(!strcmp(furi_string_get_cstr(temp_str), "RAW")) { // if RAW protocol
            subghz_protocol_raw_gen_fff_data(
                fff_data_raw, furi_string_get_cstr(file_name), subghz_devices_get_name(device));

            transmitter =
                subghz_transmitter_alloc_init(environment, furi_string_get_cstr(temp_str));
            if(transmitter == NULL) {
                printf("subghz tx_from_file: \033[0;31mError transmitter\033[0m\r\n");
                is_init_protocol = false;
            }

            if(is_init_protocol) {
                status = subghz_transmitter_deserialize(transmitter, fff_data_raw);
                if(status != SubGhzProtocolStatusOk) {
                    printf(
                        "subghz tx_from_file: \033[0;31mError deserialize protocol\033[0m %d\r\n",
                        status);
                    is_init_protocol = false;
                }
            }

        } else { //if not RAW protocol
            flipper_format_insert_or_update_uint32(fff_data_file, "Repeat", &repeat, 1);

            transmitter =
                subghz_transmitter_alloc_init(environment, furi_string_get_cstr(temp_str));
            if(transmitter == NULL) {
                printf("subghz tx_from_file: \033[0;31mError transmitter\033[0m\r\n");
                is_init_protocol = false;
            }
            if(is_init_protocol) {
                status = subghz_transmitter_deserialize(transmitter, fff_data_file);
                if(status != SubGhzProtocolStatusOk) {
                    printf(
                        "subghz tx_from_file: \033[0;31mError deserialize protocol\033[0m %d\r\n",
                        status);
                    is_init_protocol = false;
                }
            }

            flipper_format_delete_key(fff_data_file, "Repeat");
        }

        if(is_init_protocol) {
            check_file = true;
        } else {
            subghz_devices_sleep(device);
            subghz_devices_end(device);
            subghz_transmitter_free(transmitter);
        }

    } while(false);

    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    if(check_file) {
        furi_hal_power_suppress_charge_enter();

        printf(
            "Listening at \033[0;33m%s\033[0m. Frequency=%lu, Protocol=%s\r\n\r\nPress CTRL+C to stop\r\n\r\n",
            furi_string_get_cstr(file_name),
            frequency,
            furi_string_get_cstr(temp_str));
        do {
            //delay in downloading files and other preparatory processes
            furi_delay_ms(200);
            if(subghz_devices_start_async_tx(device, subghz_transmitter_yield, transmitter)) {
                while(
                    !(subghz_devices_is_async_complete_tx(device) ||
                      cli_cmd_interrupt_received(cli))) {
                    printf(".");
                    fflush(stdout);
                    furi_delay_ms(333);
                }
                subghz_devices_stop_async_tx(device);

            } else {
                printf("Transmission on this frequency is restricted in your region\r\n");
            }

            if(!strcmp(furi_string_get_cstr(temp_str), "RAW")) {
                subghz_transmitter_stop(transmitter);
                repeat--;
                if(!cli_cmd_interrupt_received(cli) && repeat)
                    subghz_transmitter_deserialize(transmitter, fff_data_raw);
            }

        } while(!cli_cmd_interrupt_received(cli) &&
                (repeat && !strcmp(furi_string_get_cstr(temp_str), "RAW")));

        subghz_devices_sleep(device);
        subghz_devices_end(device);
        subghz_cli_radio_device_power_off();

        furi_hal_power_suppress_charge_exit();

        subghz_transmitter_free(transmitter);
    }
    flipper_format_free(fff_data_raw);
    furi_string_free(file_name);
    furi_string_free(temp_str);
    subghz_devices_deinit();
    subghz_environment_free(environment);
}

static void subghz_cli_command_print_usage(void) {
    printf("Usage:\r\n");
    printf("subghz <cmd> <args>\r\n");
    printf("Cmd list:\r\n");

    printf(
        "\tchat <frequency:in Hz> <device: 0 - CC1101_INT, 1 - CC1101_EXT>\t - Chat with other Flippers\r\n");
    printf(
        "\ttx <3 byte Key: in hex> <frequency: in Hz> <te: us> <repeat: count> <device: 0 - CC1101_INT, 1 - CC1101_EXT>\t - Transmitting key\r\n");
    printf("\trx <frequency:in Hz> <device: 0 - CC1101_INT, 1 - CC1101_EXT>\t - Receive\r\n");
    printf("\trx_raw <frequency:in Hz>\t - Receive RAW\r\n");
    printf("\tdecode_raw <file_name: path_RAW_file>\t - Testing\r\n");
    printf(
        "\ttx_from_file <file_name: path_file> <repeat: count> <device: 0 - CC1101_INT, 1 - CC1101_EXT>\t - Transmitting from file\r\n");

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        printf("\r\n");
        printf("  debug cmd:\r\n");
        printf("\ttx_carrier <frequency:in Hz>\t - Transmitting carrier\r\n");
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
    uint32_t device_ind = 0; // 0 - CC1101_INT, 1 - CC1101_EXT

    if(furi_string_size(args)) {
        char* args_cstr = (char*)furi_string_get_cstr(args);
        StrintParseError parse_err = StrintParseNoError;
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &frequency, 10);
        parse_err |= strint_to_uint32(args_cstr, &args_cstr, &device_ind, 10);
        if(parse_err) {
            cli_print_usage(
                "subghz chat",
                "<Frequency: in Hz> <Device: 0 - CC1101_INT, 1 - CC1101_EXT>",
                furi_string_get_cstr(args));
            return;
        }
    }
    subghz_devices_init();
    const SubGhzDevice* device = subghz_cli_command_get_device(&device_ind);
    if(!subghz_devices_is_frequency_valid(device, frequency)) {
        printf(
            "Frequency must be in " SUBGHZ_FREQUENCY_RANGE_STR " range, not %lu\r\n", frequency);
        subghz_devices_deinit();
        subghz_cli_radio_device_power_off();
        return;
    }
    if(!furi_hal_region_is_frequency_allowed(frequency)) {
        printf(
            "In your region, only reception on this frequency (%lu) is allowed,\r\n"
            "the actual operation of the application is not possible\r\n ",
            frequency);
        return;
    }

    SubGhzChatWorker* subghz_chat = subghz_chat_worker_alloc(cli);

    if(!subghz_chat_worker_start(subghz_chat, device, frequency)) {
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

    subghz_devices_deinit();
    subghz_cli_radio_device_power_off();

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

        if(furi_string_cmp_str(cmd, "tx_from_file") == 0) {
            subghz_cli_command_tx_from_file(cli, args, context);
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

void subghz_on_system_start(void) {
#ifdef SRV_CLI
    Cli* cli = furi_record_open(RECORD_CLI);

    cli_add_command(cli, "subghz", CliCommandFlagDefault, subghz_cli_command, NULL);

    furi_record_close(RECORD_CLI);
#else
    UNUSED(subghz_cli_command);
#endif
}

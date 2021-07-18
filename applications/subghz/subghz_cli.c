#include "subghz_cli.h"

#include <furi.h>
#include <api-hal.h>
#include <stream_buffer.h>
#include <lib/subghz/protocols/subghz_protocol.h>

#define CC1101_FREQUENCY_RANGE_STR \
    "300000000...348000000 or 387000000...464000000 or 779000000...928000000"

bool subghz_check_frequency_range(uint32_t frequency) {
    if(!(frequency >= 300000000 && frequency <= 348000000) &&
       !(frequency >= 387000000 && frequency <= 464000000) &&
       !(frequency >= 779000000 && frequency <= 928000000)) {
        return false;
    }
    return true;
}

void subghz_cli_init() {
    Cli* cli = furi_record_open("cli");

    cli_add_command(
        cli, "subghz_tx_carrier", CliCommandFlagDefault, subghz_cli_command_tx_carrier, NULL);
    cli_add_command(
        cli, "subghz_rx_carrier", CliCommandFlagDefault, subghz_cli_command_rx_carrier, NULL);
    cli_add_command(cli, "subghz_tx", CliCommandFlagDefault, subghz_cli_command_tx, NULL);
    cli_add_command(cli, "subghz_rx", CliCommandFlagDefault, subghz_cli_command_rx, NULL);

    furi_record_close("cli");
}

void subghz_cli_command_tx_carrier(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 433920000;

    if(string_size(args)) {
        int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
        if(ret != 1) {
            printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
            cli_print_usage("subghz_tx_carrier", "<Frequency in HZ>", string_get_cstr(args));
            return;
        }
        if(!subghz_check_frequency_range(frequency)) {
            printf(
                "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    frequency = api_hal_subghz_set_frequency_and_path(frequency);

    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, true);

    api_hal_subghz_tx();

    printf("Transmitting at frequency %lu Hz\r\n", frequency);
    printf("Press CTRL+C to stop\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
    }

    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    api_hal_subghz_sleep();
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
        if(!subghz_check_frequency_range(frequency)) {
            printf(
                "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    frequency = api_hal_subghz_set_frequency_and_path(frequency);
    printf("Receiving at frequency %lu Hz\r\n", frequency);
    printf("Press CTRL+C to stop\r\n");

    api_hal_subghz_rx();

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
        printf("RSSI: %03.1fdbm\r", api_hal_subghz_get_rssi());
        fflush(stdout);
    }

    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    api_hal_subghz_sleep();
}

#define SUBGHZ_PT_SHORT 376
#define SUBGHZ_PT_LONG (SUBGHZ_PT_SHORT * 3)
#define SUBGHZ_PT_GUARD 10600

void subghz_cli_command_tx(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 433920000;
    size_t repeat = 10;
    uint32_t key = 0x0074BADE;

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
        if(!subghz_check_frequency_range(frequency)) {
            printf(
                "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    size_t subghz_test_data_size = 25 * 2 * sizeof(uint32_t);
    uint32_t* subghz_test_data = furi_alloc(subghz_test_data_size);

    size_t pos = 0;
    for(uint8_t i = 0; i < 24; i++) {
        uint8_t byte = i / 8;
        uint8_t bit = i % 8;
        bool value = (((uint8_t*)&key)[2 - byte] >> (7 - bit)) & 1;
        if(value) {
            subghz_test_data[pos++] = SUBGHZ_PT_SHORT;
            subghz_test_data[pos++] = SUBGHZ_PT_LONG;
        } else {
            subghz_test_data[pos++] = SUBGHZ_PT_LONG;
            subghz_test_data[pos++] = SUBGHZ_PT_SHORT;
        }
    }
    subghz_test_data[pos++] = SUBGHZ_PT_SHORT;
    subghz_test_data[pos++] = SUBGHZ_PT_SHORT + SUBGHZ_PT_GUARD;

    printf(
        "Transmitting at %lu, key %lx, repeat %u. Press CTRL+C to stop\r\n",
        frequency,
        key,
        repeat);

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    frequency = api_hal_subghz_set_frequency_and_path(frequency);

    api_hal_subghz_start_async_tx(subghz_test_data, subghz_test_data_size, repeat);
    api_hal_subghz_wait_async_tx();
    api_hal_subghz_stop_async_tx();

    free(subghz_test_data);
    api_hal_subghz_sleep();
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
    printf(string_get_cstr(text));
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
        if(!subghz_check_frequency_range(frequency)) {
            printf(
                "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n",
                frequency);
            return;
        }
    }

    // Allocate context and buffers
    SubGhzCliCommandRx* instance = furi_alloc(sizeof(SubGhzCliCommandRx));
    instance->stream = xStreamBufferCreate(sizeof(LevelDuration) * 1024, sizeof(LevelDuration));
    furi_check(instance->stream);

    SubGhzProtocol* protocol = subghz_protocol_alloc();
    subghz_protocol_load_keeloq_file(protocol, "/assets/subghz/keeloq_mfcodes");
    subghz_protocol_load_nice_flor_s_file(protocol, "/assets/subghz/nice_floor_s_rx");
    subghz_protocol_enable_dump_text(protocol, subghz_cli_command_rx_text_callback, instance);

    // Configure radio
    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    frequency = api_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    // Prepare and start RX
    api_hal_subghz_set_async_rx_callback(subghz_cli_command_rx_callback, instance);
    api_hal_subghz_start_async_rx();

    // Wait for packets to arrive
    printf("Listening at %lu. Press CTRL+C to stop\r\n", frequency);
    LevelDuration level_duration;
    while(!cli_cmd_interrupt_received(cli)) {
        int ret =
            xStreamBufferReceive(instance->stream, &level_duration, sizeof(LevelDuration), 10);
        if(ret == sizeof(LevelDuration)) {
            if(level_duration_is_reset(level_duration)) {
                printf(".");
                subghz_protocol_reset(protocol);
            } else {
                bool level = level_duration_get_level(level_duration);
                uint32_t duration = level_duration_get_duration(level_duration);
                subghz_protocol_parse(protocol, level, duration);
            }
        }
    }

    // Shutdown radio
    api_hal_subghz_stop_async_rx();
    api_hal_subghz_sleep();

    printf("\r\nPackets recieved %u\r\n", instance->packet_count);

    // Cleanup
    subghz_protocol_free(protocol);
    vStreamBufferDelete(instance->stream);
    free(instance);
}

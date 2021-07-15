#include "subghz_cli.h"

#include <furi.h>
#include <api-hal.h>
#include <stream_buffer.h>

#define CC1101_FREQUENCY_RANGE_STR \
    "300000000...348000000 or 387000000...464000000 or 779000000...928000000"

static const uint8_t subghz_test_packet_data[] = {
    0x30, // 48bytes to transmit
    0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x28,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
};

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

    cli_add_command(cli, "subghz_tx_carrier", subghz_cli_command_tx_carrier, NULL);
    cli_add_command(cli, "subghz_rx_carrier", subghz_cli_command_rx_carrier, NULL);
    cli_add_command(cli, "subghz_tx_pt", subghz_cli_command_tx_pt, NULL);
    cli_add_command(cli, "subghz_rx_pt", subghz_cli_command_rx_pt, NULL);
    cli_add_command(cli, "subghz_tx", subghz_cli_command_tx, NULL);
    cli_add_command(cli, "subghz_rx", subghz_cli_command_rx, NULL);

    furi_record_close("cli");
}

void subghz_cli_command_tx_carrier(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 0;
    int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
    if(ret != 1) {
        printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
        cli_print_usage("subghz_tx_carrier", "<Frequency in HZ>", string_get_cstr(args));
        return;
    }

    if(!subghz_check_frequency_range(frequency)) {
        printf(
            "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n", frequency);
        return;
    }

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetOokAsync);
    frequency = api_hal_subghz_set_frequency_and_path(frequency);
    printf("Transmitting at frequency %lu Hz\r\n", frequency);
    printf("Press CTRL+C to stop\r\n");

    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, false);

    api_hal_subghz_tx();

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
    }

    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    api_hal_subghz_sleep();
}

void subghz_cli_command_rx_carrier(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 0;
    int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
    if(ret != 1) {
        printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
        cli_print_usage("subghz_tx_carrier", "<Frequency in HZ>", string_get_cstr(args));
        return;
    }

    if(!subghz_check_frequency_range(frequency)) {
        printf(
            "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n", frequency);
        return;
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

void subghz_cli_command_tx_pt(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 0;
    uint32_t pattern;
    uint32_t count;

    int ret = sscanf(string_get_cstr(args), "%lu %lu %lu", &frequency, &pattern, &count);
    if(ret != 3) {
        printf(
            "sscanf returned %d, frequency: %lu; pattern: %lu; count: %lu\r\n",
            ret,
            frequency,
            pattern,
            count);
        cli_print_usage(
            "subghz_tx_pt", "<Frequency in HZ> <Pattern> <Count>", string_get_cstr(args));
        return;
    }

    if(!subghz_check_frequency_range(frequency)) {
        printf(
            "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n", frequency);
        return;
    }
    if(pattern > 1) {
        printf("Pattern must be 1, not %lu\r\n", pattern);
    }

    api_hal_subghz_reset();
    api_hal_subghz_idle();

    api_hal_subghz_load_preset(ApiHalSubGhzPreset2FskPacket);

    frequency = api_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    uint8_t status = api_hal_subghz_get_status();
    FURI_LOG_D("SUBGHZ CLI", "Status: %02X", status);

    while(!cli_cmd_interrupt_received(cli) && count) {
        api_hal_subghz_idle();
        api_hal_subghz_write_packet(subghz_test_packet_data, sizeof(subghz_test_packet_data));
        api_hal_subghz_tx();
        while(!hal_gpio_read(&gpio_cc1101_g0)) osDelay(1); // Wait for sync
        while(hal_gpio_read(&gpio_cc1101_g0)) osDelay(1); // Wait end of transaction
        count--;
    }

    api_hal_subghz_sleep();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
}

void subghz_cli_command_rx_pt(Cli* cli, string_t args, void* context) {
    uint32_t frequency = 0;

    int ret = sscanf(string_get_cstr(args), "%lu", &frequency);
    if(ret != 1) {
        printf("sscanf returned %d, frequency: %lu\r\n", ret, frequency);
        cli_print_usage("subghz_rx_pt", "<Frequency in HZ>", string_get_cstr(args));
        return;
    }

    if(!subghz_check_frequency_range(frequency)) {
        printf(
            "Frequency must be in " CC1101_FREQUENCY_RANGE_STR " range, not %lu\r\n", frequency);
        return;
    }

    api_hal_subghz_reset();
    api_hal_subghz_idle();
    api_hal_subghz_load_preset(ApiHalSubGhzPreset2FskPacket);

    frequency = api_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    uint8_t status = api_hal_subghz_get_status();
    FURI_LOG_D("SUBGHZ CLI", "Status: %02X", status);
    printf("Start receiving packets. Press CTRL+C to stop\r\n");

    api_hal_subghz_flush_rx();
    api_hal_subghz_rx();
    uint32_t packet_cnt = 0;

    while(!cli_cmd_interrupt_received(cli)) {
        if(hal_gpio_read(&gpio_cc1101_g0)) {
            while(hal_gpio_read(&gpio_cc1101_g0))
                ; // Wait reception
            packet_cnt++;
            api_hal_subghz_idle();
            api_hal_subghz_flush_rx();
            api_hal_subghz_rx();
        }
    }

    printf("Received %lu packets", packet_cnt);

    api_hal_subghz_sleep();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

#define SUBGHZ_PT_SHORT 260
#define SUBGHZ_PT_LONG (SUBGHZ_PT_SHORT * 3)
#define SUBGHZ_PT_GUARD 8060

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

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetMP);
    frequency = api_hal_subghz_set_frequency_and_path(frequency);

    api_hal_subghz_start_async_tx(subghz_test_data, subghz_test_data_size, repeat);
    api_hal_subghz_wait_async_tx();
    api_hal_subghz_stop_async_tx();

    free(subghz_test_data);
    api_hal_subghz_sleep();
}

#include <fl_subghz/protocols/subghz_protocol.h>

volatile bool subghz_cli_overrun = false;

void subghz_cli_command_rx_callback(bool level, uint32_t duration, void* context) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    LevelDuration level_duration = level_duration_make(level, duration);
    if(subghz_cli_overrun) {
        subghz_cli_overrun = false;
        level_duration = level_duration_reset();
    }
    size_t ret = xStreamBufferSendFromISR(
        context, &level_duration, sizeof(LevelDuration), &xHigherPriorityTaskWoken);
    if(sizeof(LevelDuration) != ret) subghz_cli_overrun = true;
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
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

    api_hal_subghz_reset();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetMP);
    frequency = api_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    SubGhzProtocol* protocol = subghz_protocol_alloc();
    subghz_protocol_load_keeloq_file(protocol, "/assets/subghz/keeloq_mfcodes");
    subghz_protocol_load_nice_flor_s_file(protocol, "/assets/subghz/nice_floor_s_rx");
    subghz_protocol_enable_dump_text(protocol, NULL, NULL);

    StreamBufferHandle_t rx_stream =
        xStreamBufferCreate(sizeof(LevelDuration) * 1024, sizeof(LevelDuration));

    api_hal_subghz_set_async_rx_callback(subghz_cli_command_rx_callback, rx_stream);
    api_hal_subghz_start_async_rx();

    printf("Listening at %lu. Press CTRL+C to stop\r\n", frequency);
    LevelDuration level_duration;
    while(!cli_cmd_interrupt_received(cli)) {
        int ret = xStreamBufferReceive(rx_stream, &level_duration, sizeof(LevelDuration), 10);
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

    api_hal_subghz_stop_async_rx();
    api_hal_subghz_sleep();
    subghz_protocol_free(protocol);
    vStreamBufferDelete(rx_stream);
}

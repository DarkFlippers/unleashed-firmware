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

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
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

    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);

    api_hal_subghz_rx();

    while(!cli_cmd_interrupt_received(cli)) {
        osDelay(250);
        printf("RSSI: %03.1fdbm\r", api_hal_subghz_get_rssi());
        fflush(stdout);
    }

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
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
        while(!hal_gpio_read(&gpio_cc1101_g0))
            ; // Wait for sync
        while(hal_gpio_read(&gpio_cc1101_g0))
            ; // Wait end of transaction
        count--;
    }

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
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

    api_hal_subghz_reset();
    api_hal_subghz_set_path(ApiHalSubGhzPathIsolate);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}

void subghz_cli_command_tx(Cli* cli, string_t args, void* context) {
}

#include <fl_subghz/protocols/subghz_protocol.h>

volatile bool subghz_cli_overrun = false;

void subghz_cli_command_rx_callback(
    ApiHalSubGhzCaptureLevel level,
    uint32_t duration,
    void* context) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    LevelPair pair = {.level = level, .duration = duration};
    if(subghz_cli_overrun) {
        subghz_cli_overrun = false;
        pair.level = ApiHalSubGhzCaptureLevelOverrun;
    }
    size_t ret =
        xStreamBufferSendFromISR(context, &pair, sizeof(LevelPair), &xHigherPriorityTaskWoken);
    if(sizeof(LevelPair) != ret) subghz_cli_overrun = true;
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
    api_hal_subghz_idle();
    api_hal_subghz_load_preset(ApiHalSubGhzPresetMP);

    SubGhzProtocol* protocol = subghz_protocol_alloc();
    subghz_protocol_load_keeloq_file(protocol, "/assets/subghz/keeloq_mfcodes");
    subghz_protocol_enable_dump(protocol, NULL, NULL);

    frequency = api_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);

    StreamBufferHandle_t rx_stream =
        xStreamBufferCreate(sizeof(LevelPair) * 1024, sizeof(LevelPair));

    api_hal_subghz_set_capture_callback(subghz_cli_command_rx_callback, rx_stream);
    api_hal_subghz_enable_capture();

    api_hal_subghz_flush_rx();
    api_hal_subghz_rx();

    printf("Listening at %lu. Press CTRL+C to stop\r\n", frequency);
    LevelPair pair;
    while(!cli_cmd_interrupt_received(cli)) {
        int ret = xStreamBufferReceive(rx_stream, &pair, sizeof(LevelPair), 10);
        if(ret == sizeof(LevelPair)) {
            if(pair.level == ApiHalSubGhzCaptureLevelOverrun) {
                printf(".");
                subghz_protocol_reset(protocol);
            } else {
                subghz_protocol_parse(protocol, pair);
            }
        }
    }

    subghz_protocol_free(protocol);
    vStreamBufferDelete(rx_stream);
    api_hal_subghz_disable_capture();
    api_hal_subghz_init();
}

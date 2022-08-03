#include <m-string.h>
#include <cli/cli.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <furi_hal_infrared.h>

#include "infrared_signal.h"

#define INFRARED_CLI_BUF_SIZE 10

static void infrared_cli_start_ir_rx(Cli* cli, string_t args);
static void infrared_cli_start_ir_tx(Cli* cli, string_t args);

static const struct {
    const char* cmd;
    void (*process_function)(Cli* cli, string_t args);
} infrared_cli_commands[] = {
    {.cmd = "rx", .process_function = infrared_cli_start_ir_rx},
    {.cmd = "tx", .process_function = infrared_cli_start_ir_tx},
};

static void signal_received_callback(void* context, InfraredWorkerSignal* received_signal) {
    furi_assert(received_signal);
    char buf[100];
    size_t buf_cnt;
    Cli* cli = (Cli*)context;

    if(infrared_worker_signal_is_decoded(received_signal)) {
        const InfraredMessage* message = infrared_worker_get_decoded_signal(received_signal);
        buf_cnt = snprintf(
            buf,
            sizeof(buf),
            "%s, A:0x%0*lX, C:0x%0*lX%s\r\n",
            infrared_get_protocol_name(message->protocol),
            ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4),
            message->address,
            ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
            message->command,
            message->repeat ? " R" : "");
        cli_write(cli, (uint8_t*)buf, buf_cnt);
    } else {
        const uint32_t* timings;
        size_t timings_cnt;
        infrared_worker_get_raw_signal(received_signal, &timings, &timings_cnt);

        buf_cnt = snprintf(buf, sizeof(buf), "RAW, %d samples:\r\n", timings_cnt);
        cli_write(cli, (uint8_t*)buf, buf_cnt);
        for(size_t i = 0; i < timings_cnt; ++i) {
            buf_cnt = snprintf(buf, sizeof(buf), "%lu ", timings[i]);
            cli_write(cli, (uint8_t*)buf, buf_cnt);
        }
        buf_cnt = snprintf(buf, sizeof(buf), "\r\n");
        cli_write(cli, (uint8_t*)buf, buf_cnt);
    }
}

static void infrared_cli_start_ir_rx(Cli* cli, string_t args) {
    UNUSED(cli);
    UNUSED(args);
    InfraredWorker* worker = infrared_worker_alloc();
    infrared_worker_rx_start(worker);
    infrared_worker_rx_set_received_signal_callback(worker, signal_received_callback, cli);

    printf("Receiving INFRARED...\r\nPress Ctrl+C to abort\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        furi_delay_ms(50);
    }

    infrared_worker_rx_stop(worker);
    infrared_worker_free(worker);
}

static void infrared_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("\tir rx\r\n");
    printf("\tir tx <protocol> <address> <command>\r\n");
    printf("\t<command> and <address> are hex-formatted\r\n");
    printf("\tAvailable protocols:");
    for(int i = 0; infrared_is_protocol_valid((InfraredProtocol)i); ++i) {
        printf(" %s", infrared_get_protocol_name((InfraredProtocol)i));
    }
    printf("\r\n");
    printf("\tRaw format:\r\n");
    printf("\tir tx RAW F:<frequency> DC:<duty_cycle> <sample0> <sample1>...\r\n");
    printf(
        "\tFrequency (%d - %d), Duty cycle (0 - 100), max 512 samples\r\n",
        INFRARED_MIN_FREQUENCY,
        INFRARED_MAX_FREQUENCY);
}

static bool infrared_cli_parse_message(const char* str, InfraredSignal* signal) {
    char protocol_name[32];
    InfraredMessage message;
    int parsed = sscanf(str, "%31s %lX %lX", protocol_name, &message.address, &message.command);

    if(parsed != 3) {
        return false;
    }

    message.protocol = infrared_get_protocol_by_name(protocol_name);
    message.repeat = false;
    infrared_signal_set_message(signal, &message);
    return infrared_signal_is_valid(signal);
}

static bool infrared_cli_parse_raw(const char* str, InfraredSignal* signal) {
    char frequency_str[INFRARED_CLI_BUF_SIZE];
    char duty_cycle_str[INFRARED_CLI_BUF_SIZE];
    int parsed = sscanf(str, "RAW F:%9s DC:%9s", frequency_str, duty_cycle_str);

    if(parsed != 2) {
        return false;
    }

    uint32_t* timings = malloc(sizeof(uint32_t) * MAX_TIMINGS_AMOUNT);
    uint32_t frequency = atoi(frequency_str);
    float duty_cycle = (float)atoi(duty_cycle_str) / 100;

    str += strlen(frequency_str) + strlen(duty_cycle_str) + INFRARED_CLI_BUF_SIZE;

    size_t timings_size = 0;
    while(1) {
        while(*str == ' ') {
            ++str;
        }

        char timing_str[INFRARED_CLI_BUF_SIZE];
        if(sscanf(str, "%9s", timing_str) != 1) {
            break;
        }

        str += strlen(timing_str);
        uint32_t timing = atoi(timing_str);

        if((timing <= 0) || (timings_size >= MAX_TIMINGS_AMOUNT)) {
            break;
        }

        timings[timings_size] = timing;
        ++timings_size;
    }

    infrared_signal_set_raw_signal(signal, timings, timings_size, frequency, duty_cycle);
    free(timings);

    return infrared_signal_is_valid(signal);
}

static void infrared_cli_start_ir_tx(Cli* cli, string_t args) {
    UNUSED(cli);
    const char* str = string_get_cstr(args);
    InfraredSignal* signal = infrared_signal_alloc();

    bool success = infrared_cli_parse_message(str, signal) || infrared_cli_parse_raw(str, signal);
    if(success) {
        infrared_signal_transmit(signal);
    } else {
        printf("Wrong arguments.\r\n");
        infrared_cli_print_usage();
    }

    infrared_signal_free(signal);
}

static void infrared_cli_start_ir(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    if(furi_hal_infrared_is_busy()) {
        printf("INFRARED is busy. Exiting.");
        return;
    }

    size_t i = 0;
    for(; i < COUNT_OF(infrared_cli_commands); ++i) {
        size_t size = strlen(infrared_cli_commands[i].cmd);
        bool cmd_found = !strncmp(string_get_cstr(args), infrared_cli_commands[i].cmd, size);
        if(cmd_found) {
            if(string_size(args) == size) {
                break;
            }
            if(string_get_cstr(args)[size] == ' ') {
                string_right(args, size + 1);
                break;
            }
        }
    }

    if(i < COUNT_OF(infrared_cli_commands)) {
        infrared_cli_commands[i].process_function(cli, args);
    } else {
        infrared_cli_print_usage();
    }
}
void infrared_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = (Cli*)furi_record_open(RECORD_CLI);
    cli_add_command(cli, "ir", CliCommandFlagDefault, infrared_cli_start_ir, NULL);
    furi_record_close(RECORD_CLI);
#else
    UNUSED(infrared_cli_start_ir);
#endif
}

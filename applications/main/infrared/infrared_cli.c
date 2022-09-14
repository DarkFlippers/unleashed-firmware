#include <m-string.h>
#include <cli/cli.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <furi_hal_infrared.h>
#include <flipper_format.h>
#include <toolbox/args.h>

#include "infrared_signal.h"

#define INFRARED_CLI_BUF_SIZE 10

static void infrared_cli_start_ir_rx(Cli* cli, string_t args);
static void infrared_cli_start_ir_tx(Cli* cli, string_t args);
static void infrared_cli_process_decode(Cli* cli, string_t args);

static const struct {
    const char* cmd;
    void (*process_function)(Cli* cli, string_t args);
} infrared_cli_commands[] = {
    {.cmd = "rx", .process_function = infrared_cli_start_ir_rx},
    {.cmd = "tx", .process_function = infrared_cli_start_ir_tx},
    {.cmd = "decode", .process_function = infrared_cli_process_decode},
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
    printf("\tir decode <input_file> [<output_file>]\r\n");
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

static bool
    infrared_cli_save_signal(InfraredSignal* signal, FlipperFormat* file, const char* name) {
    bool ret = infrared_signal_save(signal, file, name);
    if(!ret) {
        printf("Failed to save signal: \"%s\"\r\n", name);
    }
    return ret;
}

static bool infrared_cli_decode_raw_signal(
    InfraredRawSignal* raw_signal,
    InfraredDecoderHandler* decoder,
    FlipperFormat* output_file,
    const char* signal_name) {
    InfraredSignal* signal = infrared_signal_alloc();
    bool ret = false, level = true, is_decoded = false;

    size_t i;
    for(i = 0; i < raw_signal->timings_size; ++i) {
        // TODO: Any infrared_check_decoder_ready() magic?
        const InfraredMessage* message = infrared_decode(decoder, level, raw_signal->timings[i]);

        if(message) {
            is_decoded = true;
            printf(
                "Protocol: %s address: 0x%lX command: 0x%lX %s\r\n",
                infrared_get_protocol_name(message->protocol),
                message->address,
                message->command,
                (message->repeat ? "R" : ""));
            if(output_file && !message->repeat) {
                infrared_signal_set_message(signal, message);
                if(!infrared_cli_save_signal(signal, output_file, signal_name)) break;
            }
        }

        level = !level;
    }

    if(i == raw_signal->timings_size) {
        if(!is_decoded && output_file) {
            infrared_signal_set_raw_signal(
                signal,
                raw_signal->timings,
                raw_signal->timings_size,
                raw_signal->frequency,
                raw_signal->duty_cycle);
            ret = infrared_cli_save_signal(signal, output_file, signal_name);
        } else {
            ret = true;
        }
    }

    infrared_reset_decoder(decoder);
    infrared_signal_free(signal);
    return ret;
}

static bool infrared_cli_decode_file(FlipperFormat* input_file, FlipperFormat* output_file) {
    bool ret = false;

    InfraredSignal* signal = infrared_signal_alloc();
    InfraredDecoderHandler* decoder = infrared_alloc_decoder();

    string_t tmp;
    string_init(tmp);

    while(infrared_signal_read(signal, input_file, tmp)) {
        ret = false;
        if(!infrared_signal_is_valid(signal)) {
            printf("Invalid signal\r\n");
            break;
        }
        if(!infrared_signal_is_raw(signal)) {
            if(output_file &&
               !infrared_cli_save_signal(signal, output_file, string_get_cstr(tmp))) {
                break;
            } else {
                printf("Skipping decoded signal\r\n");
                continue;
            }
        }
        InfraredRawSignal* raw_signal = infrared_signal_get_raw_signal(signal);
        printf("Raw signal: %s, %u samples\r\n", string_get_cstr(tmp), raw_signal->timings_size);
        if(!infrared_cli_decode_raw_signal(raw_signal, decoder, output_file, string_get_cstr(tmp)))
            break;
        ret = true;
    }

    infrared_free_decoder(decoder);
    infrared_signal_free(signal);
    string_clear(tmp);

    return ret;
}

static void infrared_cli_process_decode(Cli* cli, string_t args) {
    UNUSED(cli);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* input_file = flipper_format_buffered_file_alloc(storage);
    FlipperFormat* output_file = NULL;

    uint32_t version;
    string_t tmp, header, input_path, output_path;
    string_init(tmp);
    string_init(header);
    string_init(input_path);
    string_init(output_path);

    do {
        if(!args_read_probably_quoted_string_and_trim(args, input_path)) {
            printf("Wrong arguments.\r\n");
            infrared_cli_print_usage();
            break;
        }
        args_read_probably_quoted_string_and_trim(args, output_path);
        if(!flipper_format_buffered_file_open_existing(input_file, string_get_cstr(input_path))) {
            printf("Failed to open file for reading: \"%s\"\r\n", string_get_cstr(input_path));
            break;
        }
        if(!flipper_format_read_header(input_file, header, &version) ||
           (!string_start_with_str_p(header, "IR")) || version != 1) {
            printf("Invalid or corrupted input file: \"%s\"\r\n", string_get_cstr(input_path));
            break;
        }
        if(!string_empty_p(output_path)) {
            printf("Writing output to file: \"%s\"\r\n", string_get_cstr(output_path));
            output_file = flipper_format_file_alloc(storage);
        }
        if(output_file &&
           !flipper_format_file_open_always(output_file, string_get_cstr(output_path))) {
            printf("Failed to open file for writing: \"%s\"\r\n", string_get_cstr(output_path));
            break;
        }
        if(output_file && !flipper_format_write_header(output_file, header, version)) {
            printf("Failed to write to the output file: \"%s\"\r\n", string_get_cstr(output_path));
            break;
        }
        if(!infrared_cli_decode_file(input_file, output_file)) {
            break;
        }
        printf("File successfully decoded.\r\n");
    } while(false);

    string_clear(tmp);
    string_clear(header);
    string_clear(input_path);
    string_clear(output_path);

    flipper_format_free(input_file);
    if(output_file) flipper_format_free(output_file);
    furi_record_close(RECORD_STORAGE);
}

static void infrared_cli_start_ir(Cli* cli, string_t args, void* context) {
    UNUSED(context);
    if(furi_hal_infrared_is_busy()) {
        printf("INFRARED is busy. Exiting.");
        return;
    }

    string_t command;
    string_init(command);
    args_read_string_and_trim(args, command);

    size_t i = 0;
    for(; i < COUNT_OF(infrared_cli_commands); ++i) {
        size_t cmd_len = strlen(infrared_cli_commands[i].cmd);
        if(!strncmp(string_get_cstr(command), infrared_cli_commands[i].cmd, cmd_len)) {
            break;
        }
    }

    if(i < COUNT_OF(infrared_cli_commands)) {
        infrared_cli_commands[i].process_function(cli, args);
    } else {
        infrared_cli_print_usage();
    }

    string_clear(command);
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

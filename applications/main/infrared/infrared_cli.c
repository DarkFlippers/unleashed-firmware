#include <cli/cli.h>
#include <cli/cli_i.h>
#include <infrared.h>
#include <infrared_worker.h>
#include <furi_hal_infrared.h>
#include <flipper_format.h>
#include <toolbox/args.h>

#include "infrared_signal.h"
#include "infrared_brute_force.h"

#include "m-dict.h"
#include "m-string.h"

#define INFRARED_CLI_BUF_SIZE 10

DICT_DEF2(dict_signals, string_t, STRING_OPLIST, int, M_DEFAULT_OPLIST)

enum RemoteTypes { TV = 0, AC = 1 };

static void infrared_cli_start_ir_rx(Cli* cli, FuriString* args);
static void infrared_cli_start_ir_tx(Cli* cli, FuriString* args);
static void infrared_cli_process_decode(Cli* cli, FuriString* args);
static void infrared_cli_process_universal(Cli* cli, FuriString* args);
static void infrared_cli_list_remote_signals(enum RemoteTypes remote_type);
static void
    infrared_cli_brute_force_signals(Cli* cli, enum RemoteTypes remote_type, FuriString* signal);

static const struct {
    const char* cmd;
    void (*process_function)(Cli* cli, FuriString* args);
} infrared_cli_commands[] = {
    {.cmd = "rx", .process_function = infrared_cli_start_ir_rx},
    {.cmd = "tx", .process_function = infrared_cli_start_ir_tx},
    {.cmd = "decode", .process_function = infrared_cli_process_decode},
    {.cmd = "universal", .process_function = infrared_cli_process_universal},
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

static void infrared_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("\tir rx [raw]\r\n");
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
    printf("\tir universal <tv, ac> <signal name>\r\n");
    printf("\tir universal list <tv, ac>\r\n");
}

static void infrared_cli_start_ir_rx(Cli* cli, FuriString* args) {
    UNUSED(cli);

    bool enable_decoding = true;

    if(!furi_string_empty(args)) {
        if(!furi_string_cmp_str(args, "raw")) {
            enable_decoding = false;
        } else {
            printf("Wrong arguments.\r\n");
            infrared_cli_print_usage();
            return;
        }
    }

    InfraredWorker* worker = infrared_worker_alloc();
    infrared_worker_rx_enable_signal_decoding(worker, enable_decoding);
    infrared_worker_rx_start(worker);
    infrared_worker_rx_set_received_signal_callback(worker, signal_received_callback, cli);

    printf("Receiving %s INFRARED...\r\nPress Ctrl+C to abort\r\n", enable_decoding ? "" : "RAW");
    while(!cli_cmd_interrupt_received(cli)) {
        furi_delay_ms(50);
    }

    infrared_worker_rx_stop(worker);
    infrared_worker_free(worker);
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

static void infrared_cli_start_ir_tx(Cli* cli, FuriString* args) {
    UNUSED(cli);
    const char* str = furi_string_get_cstr(args);
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

    FuriString* tmp;
    tmp = furi_string_alloc();

    while(infrared_signal_read(signal, input_file, tmp)) {
        ret = false;
        if(!infrared_signal_is_valid(signal)) {
            printf("Invalid signal\r\n");
            break;
        }
        if(!infrared_signal_is_raw(signal)) {
            if(output_file &&
               !infrared_cli_save_signal(signal, output_file, furi_string_get_cstr(tmp))) {
                break;
            } else {
                printf("Skipping decoded signal\r\n");
                continue;
            }
        }
        InfraredRawSignal* raw_signal = infrared_signal_get_raw_signal(signal);
        printf(
            "Raw signal: %s, %u samples\r\n", furi_string_get_cstr(tmp), raw_signal->timings_size);
        if(!infrared_cli_decode_raw_signal(
               raw_signal, decoder, output_file, furi_string_get_cstr(tmp)))
            break;
        ret = true;
    }

    infrared_free_decoder(decoder);
    infrared_signal_free(signal);
    furi_string_free(tmp);

    return ret;
}

static void infrared_cli_process_decode(Cli* cli, FuriString* args) {
    UNUSED(cli);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* input_file = flipper_format_buffered_file_alloc(storage);
    FlipperFormat* output_file = NULL;

    uint32_t version;
    FuriString *tmp, *header, *input_path, *output_path;
    tmp = furi_string_alloc();
    header = furi_string_alloc();
    input_path = furi_string_alloc();
    output_path = furi_string_alloc();

    do {
        if(!args_read_probably_quoted_string_and_trim(args, input_path)) {
            printf("Wrong arguments.\r\n");
            infrared_cli_print_usage();
            break;
        }
        args_read_probably_quoted_string_and_trim(args, output_path);
        if(!flipper_format_buffered_file_open_existing(
               input_file, furi_string_get_cstr(input_path))) {
            printf(
                "Failed to open file for reading: \"%s\"\r\n", furi_string_get_cstr(input_path));
            break;
        }
        if(!flipper_format_read_header(input_file, header, &version) ||
           (!furi_string_start_with_str(header, "IR")) || version != 1) {
            printf(
                "Invalid or corrupted input file: \"%s\"\r\n", furi_string_get_cstr(input_path));
            break;
        }
        if(!furi_string_empty(output_path)) {
            printf("Writing output to file: \"%s\"\r\n", furi_string_get_cstr(output_path));
            output_file = flipper_format_file_alloc(storage);
        }
        if(output_file &&
           !flipper_format_file_open_always(output_file, furi_string_get_cstr(output_path))) {
            printf(
                "Failed to open file for writing: \"%s\"\r\n", furi_string_get_cstr(output_path));
            break;
        }
        if(output_file && !flipper_format_write_header(output_file, header, version)) {
            printf(
                "Failed to write to the output file: \"%s\"\r\n",
                furi_string_get_cstr(output_path));
            break;
        }
        if(!infrared_cli_decode_file(input_file, output_file)) {
            break;
        }
        printf("File successfully decoded.\r\n");
    } while(false);

    furi_string_free(tmp);
    furi_string_free(header);
    furi_string_free(input_path);
    furi_string_free(output_path);

    flipper_format_free(input_file);
    if(output_file) flipper_format_free(output_file);
    furi_record_close(RECORD_STORAGE);
}

static void infrared_cli_process_universal(Cli* cli, FuriString* args) {
    enum RemoteTypes Remote;

    FuriString* command;
    FuriString* remote;
    FuriString* signal;
    command = furi_string_alloc();
    remote = furi_string_alloc();
    signal = furi_string_alloc();

    do {
        if(!args_read_string_and_trim(args, command)) {
            infrared_cli_print_usage();
            break;
        }

        if(furi_string_cmp_str(command, "list") == 0) {
            args_read_string_and_trim(args, remote);
            if(furi_string_cmp_str(remote, "tv") == 0) {
                Remote = TV;
            } else if(furi_string_cmp_str(remote, "ac") == 0) {
                Remote = AC;
            } else {
                printf("Invalid remote type.\r\n");
                break;
            }
            infrared_cli_list_remote_signals(Remote);
            break;
        }

        if(furi_string_cmp_str(command, "tv") == 0) {
            Remote = TV;
        } else if(furi_string_cmp_str(command, "ac") == 0) {
            Remote = AC;
        } else {
            printf("Invalid remote type.\r\n");
            break;
        }

        args_read_string_and_trim(args, signal);
        if(furi_string_empty(signal)) {
            printf("Must supply a valid signal for type of remote selected.\r\n");
            break;
        }

        infrared_cli_brute_force_signals(cli, Remote, signal);
        break;

    } while(false);

    furi_string_free(command);
    furi_string_free(remote);
    furi_string_free(signal);
}

static void infrared_cli_list_remote_signals(enum RemoteTypes remote_type) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* ff = flipper_format_buffered_file_alloc(storage);
    dict_signals_t signals_dict;
    string_t key;
    const char* remote_file = NULL;
    bool success = false;
    int max = 1;

    switch(remote_type) {
    case TV:
        remote_file = EXT_PATH("infrared/assets/tv.ir");
        break;
    case AC:
        remote_file = EXT_PATH("infrared/assets/ac.ir");
        break;
    default:
        break;
    }

    dict_signals_init(signals_dict);
    string_init(key);

    success = flipper_format_buffered_file_open_existing(ff, remote_file);
    if(success) {
        FuriString* signal_name;
        signal_name = furi_string_alloc();
        printf("Valid signals:\r\n");
        while(flipper_format_read_string(ff, "name", signal_name)) {
            string_set_str(key, furi_string_get_cstr(signal_name));
            int* v = dict_signals_get(signals_dict, key);
            if(v != NULL) {
                (*v)++;
                max = M_MAX(*v, max);
            } else {
                dict_signals_set_at(signals_dict, key, 1);
            }
        }
        dict_signals_it_t it;
        for(dict_signals_it(it, signals_dict); !dict_signals_end_p(it); dict_signals_next(it)) {
            const struct dict_signals_pair_s* pair = dict_signals_cref(it);
            printf("\t%s\r\n", string_get_cstr(pair->key));
        }
        furi_string_free(signal_name);
    }

    string_clear(key);
    dict_signals_clear(signals_dict);
    flipper_format_free(ff);
    furi_record_close(RECORD_STORAGE);
}

static void
    infrared_cli_brute_force_signals(Cli* cli, enum RemoteTypes remote_type, FuriString* signal) {
    InfraredBruteForce* brute_force = infrared_brute_force_alloc();
    const char* remote_file = NULL;
    uint32_t i = 0;
    bool success = false;

    switch(remote_type) {
    case TV:
        remote_file = EXT_PATH("infrared/assets/tv.ir");
        break;
    case AC:
        remote_file = EXT_PATH("infrared/assets/ac.ir");
        break;
    default:
        break;
    }

    infrared_brute_force_set_db_filename(brute_force, remote_file);
    infrared_brute_force_add_record(brute_force, i++, furi_string_get_cstr(signal));

    success = infrared_brute_force_calculate_messages(brute_force);
    if(success) {
        uint32_t record_count;
        uint32_t index = 0;
        int records_sent = 0;
        bool running = false;

        running = infrared_brute_force_start(brute_force, index, &record_count);
        if(record_count <= 0) {
            printf("Invalid signal.\n");
            infrared_brute_force_clear_records(brute_force);
            return;
        }

        printf("Sending %ld codes to the tv.\r\n", record_count);
        printf("Press Ctrl-C to stop.\r\n");
        while(running) {
            running = infrared_brute_force_send_next(brute_force);

            if(cli_cmd_interrupt_received(cli)) break;

            printf("\r%d%% complete.", (int)((float)records_sent++ / (float)record_count * 100));
            fflush(stdout);
        }

        infrared_brute_force_stop(brute_force);
    } else {
        printf("Invalid signal.\r\n");
    }

    infrared_brute_force_clear_records(brute_force);
    infrared_brute_force_free(brute_force);
}

static void infrared_cli_start_ir(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    if(furi_hal_infrared_is_busy()) {
        printf("INFRARED is busy. Exiting.");
        return;
    }

    FuriString* command;
    command = furi_string_alloc();
    args_read_string_and_trim(args, command);

    size_t i = 0;
    for(; i < COUNT_OF(infrared_cli_commands); ++i) {
        size_t cmd_len = strlen(infrared_cli_commands[i].cmd);
        if(!strncmp(furi_string_get_cstr(command), infrared_cli_commands[i].cmd, cmd_len)) {
            break;
        }
    }

    if(i < COUNT_OF(infrared_cli_commands)) {
        infrared_cli_commands[i].process_function(cli, args);
    } else {
        infrared_cli_print_usage();
    }

    furi_string_free(command);
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

#include <furi.h>
#include <furi_hal.h>
#include <stdarg.h>
#include <cli/cli.h>
#include <lib/toolbox/args.h>
#include <lib/lfrfid/lfrfid_worker.h>
#include <storage/storage.h>
#include <toolbox/stream/file_stream.h>

#include <toolbox/varint.h>

#include <toolbox/protocols/protocol_dict.h>
#include <lfrfid/protocols/lfrfid_protocols.h>
#include <lfrfid/lfrfid_raw_file.h>
#include <toolbox/pulse_protocols/pulse_glue.h>

static void lfrfid_cli(Cli* cli, FuriString* args, void* context);

// app cli function
void lfrfid_on_system_start(void) {
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "rfid", CliCommandFlagDefault, lfrfid_cli, NULL);
    furi_record_close(RECORD_CLI);
}

static void lfrfid_cli_print_usage(void) {
    printf("Usage:\r\n");
    printf("rfid read <optional: normal | indala>         - read in ASK/PSK mode\r\n");
    printf("rfid <write | emulate> <key_type> <key_data>  - write or emulate a card\r\n");
    printf("rfid raw_read <ask | psk> <filename>          - read and save raw data to a file\r\n");
    printf(
        "rfid raw_emulate <filename>                   - emulate raw data (not very useful, but helps debug protocols)\r\n");
    printf(
        "rfid raw_analyze <filename>                   - outputs raw data to the cli and tries to decode it (useful for protocol development)\r\n");
}

typedef struct {
    ProtocolId protocol;
    FuriEventFlag* event;
} LFRFIDCliReadContext;

static void lfrfid_cli_read_callback(LFRFIDWorkerReadResult result, ProtocolId proto, void* ctx) {
    furi_assert(ctx);
    LFRFIDCliReadContext* context = ctx;
    if(result == LFRFIDWorkerReadDone) {
        context->protocol = proto;
        FURI_SW_MEMBARRIER();
    }
    furi_event_flag_set(context->event, 1 << result);
}

static void lfrfid_cli_read(Cli* cli, FuriString* args) {
    FuriString* type_string;
    type_string = furi_string_alloc();
    LFRFIDWorkerReadType type = LFRFIDWorkerReadTypeAuto;

    if(args_read_string_and_trim(args, type_string)) {
        if(furi_string_cmp_str(type_string, "normal") == 0 ||
           furi_string_cmp_str(type_string, "ask") == 0) {
            // ask
            type = LFRFIDWorkerReadTypeASKOnly;
        } else if(
            furi_string_cmp_str(type_string, "indala") == 0 ||
            furi_string_cmp_str(type_string, "psk") == 0) {
            // psk
            type = LFRFIDWorkerReadTypePSKOnly;
        } else {
            lfrfid_cli_print_usage();
            furi_string_free(type_string);
            return;
        }
    }
    furi_string_free(type_string);

    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    LFRFIDWorker* worker = lfrfid_worker_alloc(dict);
    LFRFIDCliReadContext context;
    context.protocol = PROTOCOL_NO;
    context.event = furi_event_flag_alloc();

    lfrfid_worker_start_thread(worker);

    printf("Reading RFID...\r\nPress Ctrl+C to abort\r\n");

    const uint32_t available_flags = (1 << LFRFIDWorkerReadDone);

    lfrfid_worker_read_start(worker, type, lfrfid_cli_read_callback, &context);

    while(true) {
        uint32_t flags =
            furi_event_flag_wait(context.event, available_flags, FuriFlagWaitAny, 100);

        if(flags != (unsigned)FuriFlagErrorTimeout) {
            if(FURI_BIT(flags, LFRFIDWorkerReadDone)) {
                break;
            }
        }

        if(cli_cmd_interrupt_received(cli)) break;
    }

    lfrfid_worker_stop(worker);
    lfrfid_worker_stop_thread(worker);
    lfrfid_worker_free(worker);

    if(context.protocol != PROTOCOL_NO) {
        printf("%s ", protocol_dict_get_name(dict, context.protocol));

        size_t size = protocol_dict_get_data_size(dict, context.protocol);
        uint8_t* data = malloc(size);
        protocol_dict_get_data(dict, context.protocol, data, size);
        for(size_t i = 0; i < size; i++) {
            printf("%02X", data[i]);
        }
        printf("\r\n");
        free(data);

        FuriString* info;
        info = furi_string_alloc();
        protocol_dict_render_data(dict, info, context.protocol);
        if(!furi_string_empty(info)) {
            printf("%s\r\n", furi_string_get_cstr(info));
        }
        furi_string_free(info);
    }

    printf("Reading stopped\r\n");
    protocol_dict_free(dict);

    furi_event_flag_free(context.event);
}

static bool lfrfid_cli_parse_args(FuriString* args, ProtocolDict* dict, ProtocolId* protocol) {
    bool result = false;
    FuriString *protocol_name, *data_text;
    protocol_name = furi_string_alloc();
    data_text = furi_string_alloc();
    size_t data_size = protocol_dict_get_max_data_size(dict);
    uint8_t* data = malloc(data_size);

    do {
        // load args
        if(!args_read_string_and_trim(args, protocol_name) ||
           !args_read_string_and_trim(args, data_text)) {
            lfrfid_cli_print_usage();
            break;
        }

        // check protocol arg
        *protocol = protocol_dict_get_protocol_by_name(dict, furi_string_get_cstr(protocol_name));
        if(*protocol == PROTOCOL_NO) {
            printf(
                "Unknown protocol: %s\r\n"
                "Available protocols:\r\n",
                furi_string_get_cstr(protocol_name));

            for(ProtocolId i = 0; i < LFRFIDProtocolMax; i++) {
                printf(
                    "\t%s, %zu bytes long\r\n",
                    protocol_dict_get_name(dict, i),
                    protocol_dict_get_data_size(dict, i));
            }
            break;
        }

        data_size = protocol_dict_get_data_size(dict, *protocol);

        // check data arg
        if(!args_read_hex_bytes(data_text, data, data_size)) {
            printf(
                "%s data needs to be %zu bytes long\r\n",
                protocol_dict_get_name(dict, *protocol),
                data_size);
            break;
        }

        // load data to protocol
        protocol_dict_set_data(dict, *protocol, data, data_size);

        result = true;
    } while(false);

    free(data);
    furi_string_free(protocol_name);
    furi_string_free(data_text);
    return result;
}

static void lfrfid_cli_write_callback(LFRFIDWorkerWriteResult result, void* ctx) {
    furi_assert(ctx);
    FuriEventFlag* events = ctx;
    furi_event_flag_set(events, 1 << result);
}

static void lfrfid_cli_write(Cli* cli, FuriString* args) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    ProtocolId protocol;

    if(!lfrfid_cli_parse_args(args, dict, &protocol)) {
        protocol_dict_free(dict);
        return;
    }

    LFRFIDWorker* worker = lfrfid_worker_alloc(dict);
    FuriEventFlag* event = furi_event_flag_alloc();

    lfrfid_worker_start_thread(worker);
    lfrfid_worker_write_start(worker, protocol, lfrfid_cli_write_callback, event);

    printf("Writing RFID...\r\nPress Ctrl+C to abort\r\n");
    const uint32_t available_flags = (1 << LFRFIDWorkerWriteOK) |
                                     (1 << LFRFIDWorkerWriteProtocolCannotBeWritten) |
                                     (1 << LFRFIDWorkerWriteFobCannotBeWritten);

    while(!cli_cmd_interrupt_received(cli)) {
        uint32_t flags = furi_event_flag_wait(event, available_flags, FuriFlagWaitAny, 100);
        if(flags != (unsigned)FuriFlagErrorTimeout) {
            if(FURI_BIT(flags, LFRFIDWorkerWriteOK)) {
                printf("Written!\r\n");
                break;
            }

            if(FURI_BIT(flags, LFRFIDWorkerWriteProtocolCannotBeWritten)) {
                printf("This protocol cannot be written.\r\n");
                break;
            }

            if(FURI_BIT(flags, LFRFIDWorkerWriteFobCannotBeWritten)) {
                printf("Seems this fob cannot be written.\r\n");
            }
        }
    }
    printf("Writing stopped\r\n");

    lfrfid_worker_stop(worker);
    lfrfid_worker_stop_thread(worker);
    lfrfid_worker_free(worker);
    protocol_dict_free(dict);
    furi_event_flag_free(event);
}

static void lfrfid_cli_emulate(Cli* cli, FuriString* args) {
    ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
    ProtocolId protocol;

    if(!lfrfid_cli_parse_args(args, dict, &protocol)) {
        protocol_dict_free(dict);
        return;
    }

    LFRFIDWorker* worker = lfrfid_worker_alloc(dict);

    lfrfid_worker_start_thread(worker);
    lfrfid_worker_emulate_start(worker, protocol);

    printf("Emulating RFID...\r\nPress Ctrl+C to abort\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        furi_delay_ms(100);
    }
    printf("Emulation stopped\r\n");

    lfrfid_worker_stop(worker);
    lfrfid_worker_stop_thread(worker);
    lfrfid_worker_free(worker);
    protocol_dict_free(dict);
}

static void lfrfid_cli_raw_analyze(Cli* cli, FuriString* args) {
    UNUSED(cli);
    FuriString *filepath, *info_string;
    filepath = furi_string_alloc();
    info_string = furi_string_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);
    LFRFIDRawFile* file = lfrfid_raw_file_alloc(storage);

    do {
        float frequency = 0;
        float duty_cycle = 0;

        if(!args_read_probably_quoted_string_and_trim(args, filepath)) {
            lfrfid_cli_print_usage();
            break;
        }

        if(!lfrfid_raw_file_open_read(file, furi_string_get_cstr(filepath))) {
            printf("Failed to open file\r\n");
            break;
        }

        if(!lfrfid_raw_file_read_header(file, &frequency, &duty_cycle)) {
            printf("Invalid header\r\n");
            break;
        }

        bool file_end = false;
        uint32_t total_warns = 0;
        uint32_t total_duration = 0;
        uint32_t total_pulse = 0;
        ProtocolId total_protocol = PROTOCOL_NO;

        ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
        protocol_dict_decoders_start(dict);

        while(!file_end) {
            uint32_t pulse = 0;
            uint32_t duration = 0;
            if(lfrfid_raw_file_read_pair(file, &duration, &pulse, &file_end)) {
                bool warn = false;

                if(pulse > duration || pulse <= 0 || duration <= 0) {
                    total_warns += 1;
                    warn = true;
                }

                furi_string_printf(info_string, "[%lu %lu]", pulse, duration);
                printf("%-16s", furi_string_get_cstr(info_string));
                furi_string_printf(info_string, "[%lu %lu]", pulse, duration - pulse);
                printf("%-16s", furi_string_get_cstr(info_string));

                if(warn) {
                    printf(" <<----");
                }

                if(total_protocol == PROTOCOL_NO) {
                    total_protocol = protocol_dict_decoders_feed(dict, true, pulse);
                    if(total_protocol == PROTOCOL_NO) {
                        total_protocol =
                            protocol_dict_decoders_feed(dict, false, duration - pulse);
                    }

                    if(total_protocol != PROTOCOL_NO) {
                        printf(" <FOUND %s>", protocol_dict_get_name(dict, total_protocol));
                    }
                }

                printf("\r\n");

                total_pulse += pulse;
                total_duration += duration;

                if(total_protocol != PROTOCOL_NO) { //-V1051
                    break;
                }
            } else {
                printf("Failed to read pair\r\n");
                break;
            }
        }

        printf("   Frequency: %f\r\n", (double)frequency);
        printf("  Duty Cycle: %f\r\n", (double)duty_cycle);
        printf("       Warns: %lu\r\n", total_warns);
        printf("   Pulse sum: %lu\r\n", total_pulse);
        printf("Duration sum: %lu\r\n", total_duration);
        printf("     Average: %f\r\n", (double)((float)total_pulse / (float)total_duration));
        printf("    Protocol: ");

        if(total_protocol != PROTOCOL_NO) {
            size_t data_size = protocol_dict_get_data_size(dict, total_protocol);
            uint8_t* data = malloc(data_size);
            protocol_dict_get_data(dict, total_protocol, data, data_size);

            printf("%s [", protocol_dict_get_name(dict, total_protocol));
            for(size_t i = 0; i < data_size; i++) {
                printf("%02X", data[i]);
                if(i < data_size - 1) {
                    printf(" ");
                }
            }
            printf("]\r\n");

            protocol_dict_render_data(dict, info_string, total_protocol);
            printf("%s\r\n", furi_string_get_cstr(info_string));

            free(data);
        } else {
            printf("not found\r\n");
        }

        protocol_dict_free(dict);
    } while(false);

    furi_string_free(filepath);
    furi_string_free(info_string);
    lfrfid_raw_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static void lfrfid_cli_raw_read_callback(LFRFIDWorkerReadRawResult result, void* context) {
    furi_assert(context);
    FuriEventFlag* event = context;
    furi_event_flag_set(event, 1 << result);
}

static void lfrfid_cli_raw_read(Cli* cli, FuriString* args) {
    UNUSED(cli);

    FuriString *filepath, *type_string;
    filepath = furi_string_alloc();
    type_string = furi_string_alloc();
    LFRFIDWorkerReadType type = LFRFIDWorkerReadTypeAuto;

    do {
        if(args_read_string_and_trim(args, type_string)) {
            if(furi_string_cmp_str(type_string, "normal") == 0 ||
               furi_string_cmp_str(type_string, "ask") == 0) {
                // ask
                type = LFRFIDWorkerReadTypeASKOnly;
            } else if(
                furi_string_cmp_str(type_string, "indala") == 0 ||
                furi_string_cmp_str(type_string, "psk") == 0) {
                // psk
                type = LFRFIDWorkerReadTypePSKOnly;
            } else {
                lfrfid_cli_print_usage();
                break;
            }
        }

        if(!args_read_probably_quoted_string_and_trim(args, filepath)) {
            lfrfid_cli_print_usage();
            break;
        }

        ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
        LFRFIDWorker* worker = lfrfid_worker_alloc(dict);
        FuriEventFlag* event = furi_event_flag_alloc();

        lfrfid_worker_start_thread(worker);

        bool overrun = false;

        const uint32_t available_flags = (1 << LFRFIDWorkerReadRawFileError) |
                                         (1 << LFRFIDWorkerReadRawOverrun);

        lfrfid_worker_read_raw_start(
            worker, furi_string_get_cstr(filepath), type, lfrfid_cli_raw_read_callback, event);
        while(true) {
            uint32_t flags = furi_event_flag_wait(event, available_flags, FuriFlagWaitAny, 100);

            if(flags != (unsigned)FuriFlagErrorTimeout) {
                if(FURI_BIT(flags, LFRFIDWorkerReadRawFileError)) {
                    printf("File is not RFID raw file\r\n");
                    break;
                }

                if(FURI_BIT(flags, LFRFIDWorkerReadRawOverrun)) {
                    if(!overrun) {
                        printf("Overrun\r\n");
                        overrun = true;
                    }
                }
            }

            if(cli_cmd_interrupt_received(cli)) break;
        }

        if(overrun) {
            printf("An overrun occurred during read\r\n");
        }

        lfrfid_worker_stop(worker);

        lfrfid_worker_stop_thread(worker);
        lfrfid_worker_free(worker);
        protocol_dict_free(dict);

        furi_event_flag_free(event);

    } while(false);

    furi_string_free(filepath);
    furi_string_free(type_string);
}

static void lfrfid_cli_raw_emulate_callback(LFRFIDWorkerEmulateRawResult result, void* context) {
    furi_assert(context);
    FuriEventFlag* event = context;
    furi_event_flag_set(event, 1 << result);
}

static void lfrfid_cli_raw_emulate(Cli* cli, FuriString* args) {
    UNUSED(cli);

    FuriString* filepath;
    filepath = furi_string_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    do {
        if(!args_read_probably_quoted_string_and_trim(args, filepath)) {
            lfrfid_cli_print_usage();
            break;
        }

        if(!storage_file_exists(storage, furi_string_get_cstr(filepath))) {
            printf("File not found: \"%s\"\r\n", furi_string_get_cstr(filepath));
            break;
        }

        ProtocolDict* dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);
        LFRFIDWorker* worker = lfrfid_worker_alloc(dict);
        FuriEventFlag* event = furi_event_flag_alloc();

        lfrfid_worker_start_thread(worker);

        bool overrun = false;

        const uint32_t available_flags = (1 << LFRFIDWorkerEmulateRawFileError) |
                                         (1 << LFRFIDWorkerEmulateRawOverrun);

        lfrfid_worker_emulate_raw_start(
            worker, furi_string_get_cstr(filepath), lfrfid_cli_raw_emulate_callback, event);
        while(true) {
            uint32_t flags = furi_event_flag_wait(event, available_flags, FuriFlagWaitAny, 100);

            if(flags != (unsigned)FuriFlagErrorTimeout) {
                if(FURI_BIT(flags, LFRFIDWorkerEmulateRawFileError)) {
                    printf("File is not RFID raw file\r\n");
                    break;
                }

                if(FURI_BIT(flags, LFRFIDWorkerEmulateRawOverrun)) {
                    if(!overrun) {
                        printf("Overrun\r\n");
                        overrun = true;
                    }
                }
            }

            if(cli_cmd_interrupt_received(cli)) break;
        }

        if(overrun) {
            printf("An overrun occurred during emulation\r\n");
        }

        lfrfid_worker_stop(worker);

        lfrfid_worker_stop_thread(worker);
        lfrfid_worker_free(worker);
        protocol_dict_free(dict);

        furi_event_flag_free(event);

    } while(false);

    furi_record_close(RECORD_STORAGE);
    furi_string_free(filepath);
}

static void lfrfid_cli(Cli* cli, FuriString* args, void* context) {
    UNUSED(context);
    FuriString* cmd;
    cmd = furi_string_alloc();

    if(!args_read_string_and_trim(args, cmd)) {
        furi_string_free(cmd);
        lfrfid_cli_print_usage();
        return;
    }

    if(furi_string_cmp_str(cmd, "read") == 0) {
        lfrfid_cli_read(cli, args);
    } else if(furi_string_cmp_str(cmd, "write") == 0) {
        lfrfid_cli_write(cli, args);
    } else if(furi_string_cmp_str(cmd, "emulate") == 0) {
        lfrfid_cli_emulate(cli, args);
    } else if(furi_string_cmp_str(cmd, "raw_read") == 0) {
        lfrfid_cli_raw_read(cli, args);
    } else if(furi_string_cmp_str(cmd, "raw_emulate") == 0) {
        lfrfid_cli_raw_emulate(cli, args);
    } else if(furi_string_cmp_str(cmd, "raw_analyze") == 0) {
        lfrfid_cli_raw_analyze(cli, args);
    } else {
        lfrfid_cli_print_usage();
    }

    furi_string_free(cmd);
}

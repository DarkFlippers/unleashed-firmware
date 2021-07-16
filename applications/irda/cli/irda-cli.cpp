#include <api-hal-delay.h>
#include <irda.h>
#include <app-template.h>
#include <cli/cli.h>
#include <cmsis_os2.h>
#include <irda_worker.h>
#include <furi.h>
#include <api-hal-irda.h>
#include <sstream>
#include <string>
#include <m-string.h>
#include <irda_transmit.h>

static void signal_received_callback(void* context, IrdaWorkerSignal* received_signal) {
    furi_assert(received_signal);
    char buf[100];
    size_t buf_cnt;
    Cli* cli = (Cli*)context;

    if(irda_worker_signal_is_decoded(received_signal)) {
        const IrdaMessage* message = irda_worker_get_decoded_message(received_signal);
        buf_cnt = sniprintf(
            buf,
            sizeof(buf),
            "%s, A:0x%0*lX, C:0x%0*lX%s\r\n",
            irda_get_protocol_name(message->protocol),
            irda_get_protocol_address_length(message->protocol),
            message->address,
            irda_get_protocol_command_length(message->protocol),
            message->command,
            message->repeat ? " R" : "");
        cli_write(cli, (uint8_t*)buf, buf_cnt);
    } else {
        const uint32_t* timings;
        size_t timings_cnt;
        irda_worker_get_raw_signal(received_signal, &timings, &timings_cnt);

        buf_cnt = sniprintf(buf, sizeof(buf), "RAW, %d samples:\r\n", timings_cnt);
        cli_write(cli, (uint8_t*)buf, buf_cnt);
        for(size_t i = 0; i < timings_cnt; ++i) {
            buf_cnt = sniprintf(buf, sizeof(buf), "%lu ", timings[i]);
            cli_write(cli, (uint8_t*)buf, buf_cnt);
        }
        buf_cnt = sniprintf(buf, sizeof(buf), "\r\n");
        cli_write(cli, (uint8_t*)buf, buf_cnt);
    }
}

static void irda_cli_start_ir_rx(Cli* cli, string_t args, void* context) {
    if(api_hal_irda_rx_irq_is_busy()) {
        printf("IRDA is busy. Exit.");
        return;
    }

    IrdaWorker* worker = irda_worker_alloc();
    irda_worker_set_context(worker, cli);
    irda_worker_start(worker);
    irda_worker_set_received_signal_callback(worker, signal_received_callback);

    printf("Receiving IRDA...\r\nPress Ctrl+C to abort\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        delay(50);
    }

    irda_worker_stop(worker);
    irda_worker_free(worker);
}

static void irda_cli_print_usage(void) {
    printf("Usage:\r\n\tir_tx <protocol> <address> <command>\r\n");
    printf("\t<command> and <address> are hex-formatted\r\n");
    printf("\tAvailable protocols:");
    for(int i = 0; irda_is_protocol_valid((IrdaProtocol)i); ++i) {
        printf(" %s", irda_get_protocol_name((IrdaProtocol)i));
    }
    printf("\r\n");
}

static bool parse_message(const char* str, IrdaMessage* message) {
    uint32_t command = 0;
    uint32_t address = 0;
    char protocol_name[32];
    int parsed = sscanf(str, "%31s %lX %lX", protocol_name, &address, &command);

    if(parsed != 3) {
        return false;
    }

    IrdaProtocol protocol = irda_get_protocol_by_name(protocol_name);

    if(!irda_is_protocol_valid(protocol)) {
        return false;
    }

    message->protocol = protocol;
    message->address = address;
    message->command = command;
    message->repeat = false;

    return true;
}

static bool parse_signal_raw(
    const char* str,
    uint32_t* timings,
    uint32_t* timings_cnt,
    float* duty_cycle,
    float* frequency) {
    char frequency_str[10];
    char duty_cycle_str[10];
    int parsed = sscanf(str, "RAW F:%9s DC:%9s", frequency_str, duty_cycle_str);
    if(parsed != 2) return false;

    *frequency = atoi(frequency_str);
    *duty_cycle = (float)atoi(duty_cycle_str) / 100;
    str += strlen(frequency_str) + strlen(duty_cycle_str) + 10;

    uint32_t timings_cnt_max = *timings_cnt;
    *timings_cnt = 0;

    while(1) {
        char timing_str[10];
        for(; *str == ' '; ++str)
            ;
        if(1 != sscanf(str, "%9s", timing_str)) break;
        str += strlen(timing_str);
        uint32_t timing = atoi(timing_str);
        if(timing <= 0) break;
        if(*timings_cnt >= timings_cnt_max) break;
        timings[*timings_cnt] = timing;
        ++*timings_cnt;
    }

    printf("\r\nTransmit:");
    for(size_t i = 0; i < *timings_cnt; ++i) {
        printf(" %ld", timings[i]);
    }
    printf("\r\n");

    return true;
}

static void irda_cli_start_ir_tx(Cli* cli, string_t args, void* context) {
    if(api_hal_irda_rx_irq_is_busy()) {
        printf("IRDA is busy. Exit.");
        return;
    }

    IrdaMessage message;
    const char* str = string_get_cstr(args);
    float frequency;
    float duty_cycle;
    uint32_t* timings = (uint32_t*)furi_alloc(sizeof(uint32_t) * 1000);
    uint32_t timings_cnt = 1000;

    if(parse_message(str, &message)) {
        irda_send(&message, 1);
    } else if(parse_signal_raw(str, timings, &timings_cnt, &duty_cycle, &frequency)) {
        irda_send_raw_ext(timings, timings_cnt, true, duty_cycle, frequency);
    } else {
        printf("Wrong arguments.\r\n");
        irda_cli_print_usage();
    }

    free(timings);
}

extern "C" void irda_cli_init() {
    Cli* cli = (Cli*)furi_record_open("cli");
    cli_add_command(cli, "ir_rx", irda_cli_start_ir_rx, NULL);
    cli_add_command(cli, "ir_tx", irda_cli_start_ir_tx, NULL);
    furi_record_close("cli");
}

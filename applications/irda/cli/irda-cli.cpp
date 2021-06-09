#include "app-template.h"
#include "cli/cli.h"
#include "cmsis_os2.h"
#include <furi.h>
#include <api-hal-irda.h>
#include "irda.h"
#include <sstream>
#include <string>
#include <m-string.h>

typedef struct IrdaCli {
    IrdaHandler* handler;
    osMessageQueueId_t message_queue;
} IrdaCli;

static void irda_rx_callback(void* ctx, bool level, uint32_t duration) {
    IrdaCli* irda_cli = (IrdaCli*)ctx;
    const IrdaMessage* message;
    message = irda_decode(irda_cli->handler, level, duration);
    if(message) {
        osMessageQueuePut(irda_cli->message_queue, message, 0, 0);
    }
}

static void irda_cli_start_ir_rx(Cli* cli, string_t args, void* context) {
    if(api_hal_irda_rx_irq_is_busy()) {
        printf("IRDA is busy. Exit.");
        return;
    }
    IrdaCli irda_cli;
    irda_cli.handler = irda_alloc_decoder();
    irda_cli.message_queue = osMessageQueueNew(2, sizeof(IrdaMessage), NULL);
    api_hal_irda_rx_irq_init();
    api_hal_irda_rx_irq_set_callback(irda_rx_callback, &irda_cli);

    printf("Receiving IRDA...\r\nPress Ctrl+C to abort\r\n");
    while(!cli_cmd_interrupt_received(cli)) {
        IrdaMessage message;
        if(osOK == osMessageQueueGet(irda_cli.message_queue, &message, NULL, 50)) {
            printf(
                "%s, A:0x%0*lX, C:0x%0*lX%s\r\n",
                irda_get_protocol_name(message.protocol),
                irda_get_protocol_address_length(message.protocol),
                message.address,
                irda_get_protocol_command_length(message.protocol),
                message.command,
                message.repeat ? " R" : "");
        }
    }

    api_hal_irda_rx_irq_deinit();
    irda_free_decoder(irda_cli.handler);
    osMessageQueueDelete(irda_cli.message_queue);
}

static void irda_cli_print_usage(void) {
    printf("Usage:\r\n\tir_tx <protocol> <command> <address>\r\n");
    printf("\t<command> and <address> are hex-formatted\r\n");
    printf("\tAvailable protocols:");
    for(int i = 0; irda_is_protocol_valid((IrdaProtocol)i); ++i) {
        printf(" %s", irda_get_protocol_name((IrdaProtocol)i));
    }
    printf("\r\n");
}

static void irda_cli_start_ir_tx(Cli* cli, string_t args, void* context) {
    if(api_hal_irda_rx_irq_is_busy()) {
        printf("IRDA is busy. Exit.");
        return;
    }
    auto ss = std::istringstream(string_get_cstr(args));
    uint32_t command = 0;
    uint32_t address = 0;
    std::string protocol_name;

    if(!(ss >> protocol_name) || !(ss >> std::hex >> address) || !(ss >> std::hex >> command)) {
        printf("Wrong arguments.\r\n");
        irda_cli_print_usage();
        return;
    }

    IrdaProtocol protocol = irda_get_protocol_by_name(protocol_name.c_str());

    if(!irda_is_protocol_valid(protocol)) {
        printf("Unknown protocol.\r\n");
        irda_cli_print_usage();
        return;
    }

    IrdaMessage message = {
        .protocol = protocol,
        .address = address,
        .command = command,
        .repeat = false,
    };
    irda_send(&message, 1);
}

extern "C" void irda_cli_init() {
    Cli* cli = (Cli*)furi_record_open("cli");
    cli_add_command(cli, "ir_rx", irda_cli_start_ir_rx, NULL);
    cli_add_command(cli, "ir_tx", irda_cli_start_ir_tx, NULL);
    furi_record_close("cli");
}

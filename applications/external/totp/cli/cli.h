#pragma once

#include <cli/cli.h>
#include "../types/plugin_state.h"

typedef struct {
    PluginState* plugin_state;
    FuriMessageQueue* event_queue;
} TotpCliContext;

TotpCliContext*
    totp_cli_register_command_handler(PluginState* plugin_state, FuriMessageQueue* event_queue);
void totp_cli_unregister_command_handler(TotpCliContext* context);
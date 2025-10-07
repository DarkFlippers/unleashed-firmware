#pragma once
#include <furi.h>
#include <nfc/nfc.h>
#include "nfc_cli_command_base.h"

typedef struct NfcCliProcessorContext NfcCliProcessorContext;

NfcCliProcessorContext* nfc_cli_command_processor_alloc(Nfc* nfc);
void nfc_cli_command_processor_free(NfcCliProcessorContext* instance);

void nfc_cli_command_processor_run(
    const NfcCliCommandDescriptor* cmd,
    PipeSide* pipe,
    FuriString* args,
    void* context);

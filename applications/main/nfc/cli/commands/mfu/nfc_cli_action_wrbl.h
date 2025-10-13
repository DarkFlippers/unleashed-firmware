#pragma once

#include "../../nfc_cli_command_base_i.h"

NfcCliActionContext* nfc_cli_mfu_wrbl_alloc_ctx(Nfc* nfc);
void nfc_cli_mfu_wrbl_free_ctx(NfcCliActionContext* ctx);
void nfc_cli_mfu_wrbl_execute(PipeSide* pipe, NfcCliActionContext* ctx);
bool nfc_cli_mfu_wrbl_parse_block(FuriString* value, NfcCliActionContext* ctx);
bool nfc_cli_mfu_wrbl_parse_data(FuriString* value, void* output);

#pragma once

#include "../../nfc_cli_command_base_i.h"

NfcCliActionContext* nfc_cli_mfu_rdbl_alloc_ctx(Nfc* nfc);
void nfc_cli_mfu_rdbl_free_ctx(NfcCliActionContext* ctx);
void nfc_cli_mfu_rdbl_execute(PipeSide* pipe, NfcCliActionContext* ctx);
bool nfc_cli_mfu_rdbl_parse_block(FuriString* value, NfcCliActionContext* ctx);

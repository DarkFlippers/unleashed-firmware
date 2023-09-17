#pragma once

#include <furi_hal_rtc.h>
#include <nfc/helpers/bit_lib.h>
#include <nfc_worker_i.h>

bool parse_transport_block(MfClassicBlock* block, FuriString* result);
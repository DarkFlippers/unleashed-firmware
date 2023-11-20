#pragma once

#include <furi_hal_rtc.h>
#include <nfc/helpers/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic.h>

bool parse_transport_block(const MfClassicBlock* block, FuriString* result);
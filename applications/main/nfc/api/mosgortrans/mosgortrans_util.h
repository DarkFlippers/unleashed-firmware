#pragma once

#include <bit_lib.h>
#include <datetime.h>
#include <furi/core/string.h>
#include <nfc/protocols/mf_classic/mf_classic.h>
#include <furi_hal_rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

void render_section_header(
    FuriString* str,
    const char* name,
    uint8_t prefix_separator_cnt,
    uint8_t suffix_separator_cnt);
bool mosgortrans_parse_transport_block(const MfClassicBlock* block, FuriString* result);

#ifdef __cplusplus
}
#endif

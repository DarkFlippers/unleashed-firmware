#pragma once

#include "ntag4xx.h"

#include <nfc/helpers/nxp_native_command.h>

#define NTAG4XX_FFF_PICC_PREFIX "PICC"

// Internal helpers

Ntag4xxError ntag4xx_process_error(Iso14443_4aError error);

Ntag4xxError ntag4xx_process_status_code(uint8_t status_code);

// Parse internal Ntag4xx structures

bool ntag4xx_version_parse(Ntag4xxVersion* data, const BitBuffer* buf);

// Load internal Ntag4xx structures

bool ntag4xx_version_load(Ntag4xxVersion* data, FlipperFormat* ff);

// Save internal Ntag4xx structures

bool ntag4xx_version_save(const Ntag4xxVersion* data, FlipperFormat* ff);

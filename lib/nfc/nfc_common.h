/**
 * @file nfc_common.h
 * @brief Various common NFC-related macros.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* NFC file format version which changed ATQA format. Deprecated. */
#define NFC_LSB_ATQA_FORMAT_VERSION          (2)
/* NFC file format version which is still supported as backwards compatible. */
#define NFC_MINIMUM_SUPPORTED_FORMAT_VERSION NFC_LSB_ATQA_FORMAT_VERSION
/* NFC file format version which implemented the unified loading process. */
#define NFC_UNIFIED_FORMAT_VERSION           (4)
/* Current NFC file format version. */
#define NFC_CURRENT_FORMAT_VERSION           NFC_UNIFIED_FORMAT_VERSION

#ifdef __cplusplus
}
#endif

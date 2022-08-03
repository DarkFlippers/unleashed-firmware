/**
 * @file event_flag.h
 * Furi Event Flag
 */
#pragma once

#include "base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void FuriEventFlag;

/** Allocate FuriEventFlag
 *
 * @return     pointer to FuriEventFlag
 */
FuriEventFlag* furi_event_flag_alloc();

/** Deallocate FuriEventFlag
 *
 * @param      instance  pointer to FuriEventFlag
 */
void furi_event_flag_free(FuriEventFlag* instance);

/** Set flags
 *
 * @param      instance  pointer to FuriEventFlag
 * @param[in]  flags     The flags
 *
 * @return     Resulting flags or error (FuriStatus)
 */
uint32_t furi_event_flag_set(FuriEventFlag* instance, uint32_t flags);

/** Clear flags
 *
 * @param      instance  pointer to FuriEventFlag
 * @param[in]  flags     The flags
 *
 * @return     Resulting flags or error (FuriStatus)
 */
uint32_t furi_event_flag_clear(FuriEventFlag* instance, uint32_t flags);

/** Get flags
 *
 * @param      instance  pointer to FuriEventFlag
 *
 * @return     Resulting flags
 */
uint32_t furi_event_flag_get(FuriEventFlag* instance);

/** Wait flags
 *
 * @param      instance  pointer to FuriEventFlag
 * @param[in]  flags     The flags
 * @param[in]  options   The option flags
 * @param[in]  timeout   The timeout
 *
 * @return     Resulting flags or error (FuriStatus)
 */
uint32_t furi_event_flag_wait(
    FuriEventFlag* instance,
    uint32_t flags,
    uint32_t options,
    uint32_t timeout);

#ifdef __cplusplus
}
#endif

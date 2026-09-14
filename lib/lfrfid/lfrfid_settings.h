/** @file lfrfid_settings.h
 *
 * Persistent LFRFID settings. Used by the app and the CLI, and readable by any app that wants
 * to honour the user's choice - LFRFIDWorker itself starts at the default mask, so honouring
 * it is opt-in, see lfrfid_worker_set_write_targets().
 *
 * Scalars rather than a struct on purpose: the stored layout is private to the firmware, so
 * adding a setting later cannot disturb an app built against an older SDK.
 */

#pragma once
#include "lfrfid_write_targets.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Write targets the user allows.
 *
 * Never fails: falls back to lfrfid_write_targets_default() when the settings file is
 * missing or was written by another version.
 *
 * @return     mask of LFRFIDWriteTarget bits
 */
LFRFIDWriteTargetMask lfrfid_settings_get_write_targets(void);

/** Persist the write targets the user allows.
 *
 * @param      mask  Mask of LFRFIDWriteTarget bits
 * @return     true on success
 */
bool lfrfid_settings_set_write_targets(LFRFIDWriteTargetMask mask);

#ifdef __cplusplus
}
#endif

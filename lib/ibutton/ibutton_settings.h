/** @file ibutton_settings.h
 *
 * Persistent iButton settings. Used by the app and the CLI, and readable by any app that wants
 * to honour the user's choice - iButtonWorker itself starts at the default mask, so honouring
 * it is opt-in, see ibutton_worker_set_write_targets().
 *
 * Scalars rather than a struct on purpose: the stored layout is private to the firmware, so
 * adding a setting later cannot disturb an app built against an older SDK.
 */

#pragma once

#include "ibutton_write_targets.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Write targets the user allows.
 *
 * Never fails: falls back to ibutton_write_targets_default() when the settings file is missing
 * or was written by another version.
 *
 * @return     mask of iButtonWriteTarget bits
 */
iButtonWriteTargetMask ibutton_settings_get_write_targets(void);

/** Persist the write targets the user allows.
 *
 * @param      mask  Mask of iButtonWriteTarget bits
 * @return     true on success
 */
bool ibutton_settings_set_write_targets(iButtonWriteTargetMask mask);

#ifdef __cplusplus
}
#endif

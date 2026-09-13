/** @file lfrfid_write_targets.h
 *
 * Writable chips a key can be cloned onto.
 *
 * LFRFIDWriteType says how a write is encoded; a target additionally pins down which chip
 * is being addressed, since the ID82xx / Hitag micro family shares one encoding but needs a
 * different password per variant.
 *
 * Targets are what the user enables or disables in settings, and the enum value is the bit
 * position in the saved mask - so append new ones at the end, and see lfrfid_settings.c for
 * what that means for an existing settings file.
 */

#pragma once
#include <toolbox/protocols/protocol_dict.h>
#include "protocols/lfrfid_protocols.h"
#include "tools/hitagmicro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LFRFIDWriteTargetT5577,
    LFRFIDWriteTargetEM4305,
    LFRFIDWriteTargetHitagMicro8265,
    LFRFIDWriteTargetHitagMicro8210,
    LFRFIDWriteTargetHitagMicroH55,

    LFRFIDWriteTargetMax,
} LFRFIDWriteTarget;

/** A set of write targets, one bit per LFRFIDWriteTarget. */
typedef uint32_t LFRFIDWriteTargetMask;

/** The default mask. */
#define LFRFID_WRITE_TARGET_MASK_ALL ((LFRFIDWriteTargetMask)((1UL << LFRFIDWriteTargetMax) - 1))

/** Bit this target occupies in a mask. */
#define LFRFID_WRITE_TARGET_BIT(target) ((LFRFIDWriteTargetMask)(1UL << (target)))

/** How data for this target is encoded. Firmware internal, not exported to apps.
 *
 * @param      target  The write target
 * @return     the write type to fill a LFRFIDWriteRequest with
 */
LFRFIDWriteType lfrfid_write_target_type(LFRFIDWriteTarget target);

/** Chip variant this target addresses, for LFRFIDWriteTypeHitagMicro targets only.
 * Firmware internal, not exported to apps.
 *
 * @param      target  The write target, which must be a Hitag micro one
 * @return     the variant whose password unlocks this chip
 */
HitagMicroVariant lfrfid_write_target_variant(LFRFIDWriteTarget target);

/** Chip name, as shown on the write screen and in settings, e.g. "T5577" or "8210".
 *
 * @param      target  The write target
 * @return     pointer to a static string
 */
const char* lfrfid_write_target_name(LFRFIDWriteTarget target);

/** Mask of the targets a protocol can be written to.
 *
 * Probes the protocol exactly as the write loop does, so it leaves the protocol's data
 * modified: snapshot it with protocol_dict_get_data() first if the caller still needs it.
 *
 * @param      dict      The protocol dictionary
 * @param      protocol  The protocol to probe
 * @return     mask of LFRFIDWriteTarget bits, 0 if the protocol cannot be written at all
 */
LFRFIDWriteTargetMask lfrfid_write_targets_supported(ProtocolDict* dict, ProtocolId protocol);

#ifdef __cplusplus
}
#endif

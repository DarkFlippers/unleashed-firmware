/** @file ibutton_write_targets.h
 *
 * Blank chip types a key can be written onto.
 *
 * A Dallas key carries no hint of which blank is in front of the reader, so a write attempt
 * tries each enabled target in turn and keeps the one whose read-back matches. A target is what
 * the user enables or disables in settings, and the enum value is the bit position in the saved
 * mask - so append new ones at the end, and see ibutton_settings.c for what that means for an
 * existing settings file.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <one_wire/one_wire_host.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    iButtonWriteTargetRW1990_1,
    iButtonWriteTargetRW1990_2,
    iButtonWriteTargetTM2004,
    iButtonWriteTargetTM01x,

    iButtonWriteTargetMax,
} iButtonWriteTarget;

/** A set of write targets, one bit per iButtonWriteTarget. */
typedef uint32_t iButtonWriteTargetMask;

/** Every target. Use it to validate a mask, not to build one - see ibutton_write_targets_default(). */
#define IBUTTON_WRITE_TARGET_MASK_ALL \
    ((iButtonWriteTargetMask)((1UL << iButtonWriteTargetMax) - 1))

/** Bit this target occupies in a mask. */
#define IBUTTON_WRITE_TARGET_BIT(target) ((iButtonWriteTargetMask)(1UL << (target)))

/** Targets enabled when the user has expressed no preference.
 *
 * @return     mask of iButtonWriteTarget bits
 */
iButtonWriteTargetMask ibutton_write_targets_default(void);

/** Human readable name, for the write screen and the settings list.
 *
 * @param      target  The write target
 * @return     a static string, never NULL
 */
const char* ibutton_write_target_name(iButtonWriteTarget target);

/** What a write attempt may address, and who to tell about each try.
 *
 * target_cb runs on the worker thread and must not be called with the scheduler masked. It
 * may block, to let a listener repaint before the attempt starts.
 */
typedef struct {
    iButtonWriteTargetMask mask;
    void (*target_cb)(iButtonWriteTarget target, void* context);
    void* context;
} iButtonWriteTargetContext;

/** Write a ROM onto one specific blank type and verify it by reading back.
 *
 * Runs the whole command sequence for that blank, so the caller is responsible for the bus
 * being idle and for not interleaving anything else with it.
 *
 * @param      host       OneWireHost instance
 * @param      target     Blank type to address
 * @param      data       ROM bytes to write
 * @param      data_size  Number of bytes
 * @return     true if the blank accepted the data and read it back unchanged
 */
bool ibutton_write_target_write(
    OneWireHost* host,
    iButtonWriteTarget target,
    const uint8_t* data,
    size_t data_size);

#ifdef __cplusplus
}
#endif

#pragma once

#include <core/kernel.h>
#include <one_wire/one_wire_host.h>

/*
 * A TM01x blank accepts a Dallas ID, a Cyfral code and a Metakom code.
 *
 * A Dallas ID is written as raw ROM bytes and is never finalized: no finalize
 * command is known for it.
 *
 * Cyfral and Metakom are written as a 36-bit frame with a 4-bit prefix and are
 * finalized into the matching mode. The frame is read back and compared before
 * the blank is finalized, so a blank that fails verification stays writable.
 * Finalizing is one-way: a finalized blank no longer answers 0x33 and cannot be
 * written again, not even as a Dallas key. A finalize command that was sent but
 * not acknowledged leaves the blank in an unknown state, to be treated as spent.
 */

/**
 * Write a Dallas (DS1990) ROM code into a TM01x blank.
 *
 * @param host       OneWire host instance.
 * @param data       ROM data, exactly 8 bytes.
 * @param data_size  Must be 8.
 *
 * @returns true if the data was written and read back without differences.
 */
bool tm01x_write_dallas(OneWireHost* host, const uint8_t* data, size_t data_size);

/**
 * Write a Cyfral code into a TM01x blank and finalize it.
 *
 * @param host  OneWire host instance.
 * @param code  32 code bits of the frame, most significant bit first, in the
 *              order they go on the wire.
 *
 * @returns true if the code was written, read back without differences and the
 *          finalize command was sent.
 */
bool tm01x_write_cyfral(OneWireHost* host, uint32_t code);

/**
 * Write a Metakom code into a TM01x blank and finalize it.
 *
 * @param host  OneWire host instance.
 * @param code  32 code bits of the frame, most significant bit first, in the
 *              order they go on the wire.
 *
 * @returns true if the code was written, read back without differences and the
 *          finalize command was sent.
 */
bool tm01x_write_metakom(OneWireHost* host, uint32_t code);

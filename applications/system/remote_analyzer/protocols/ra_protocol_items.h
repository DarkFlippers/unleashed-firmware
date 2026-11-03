/**
 * @file ra_protocol_items.h
 * @brief The protocol set the receiver decodes against.
 *
 * The firmware's own set comes first and is used as-is - it is already in flash, so there is
 * nothing to duplicate - followed by this app's decoder-only protocols. Only the pointer
 * table is allocated; the decoders themselves are allocated once by SubGhzReceiver.
 */
#pragma once

#include <lib/subghz/registry.h>

/** Allocate the combined registry. Free with ra_protocol_registry_free(). */
const SubGhzProtocolRegistry* ra_protocol_registry_alloc(void);

/** True when this protocol is one the app adds, so the SubGhz app knows nothing about it. */
bool ra_protocol_is_own(const SubGhzProtocol* protocol);

void ra_protocol_registry_free(const SubGhzProtocolRegistry* registry);

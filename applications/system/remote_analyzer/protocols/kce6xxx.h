/**
 * @file kce6xxx.h
 * @brief KCE6xxx remote, decoder only.
 *
 * 315MHz OOK. A 3.35ms carrier-off header, then 65 pulses: 32 (high, low) bit cells at
 * 175/500us plus one short trailing high. Long high with short low is 1, the reverse is 0.
 *
 * Derived from 7 captures, ~60 identical frames per press. The 32 bits differ completely
 * between presses and no fixed serial field was found, so the whole value is shown as one key.
 */
#pragma once

#include <lib/subghz/protocols/base.h>

#define SUBGHZ_PROTOCOL_KCE6XXX_NAME "KCE6xxx"

extern const SubGhzProtocol ra_protocol_kce6xxx;

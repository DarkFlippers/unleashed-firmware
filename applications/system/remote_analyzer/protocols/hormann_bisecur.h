/**
 * @file hormann_bisecur.h
 * @brief Hormann BiSecur remote, decoder only.
 *
 * 176 bit Manchester frame at 868MHz FM, 208/416us symbols. Ported from Momentum
 * (lib/subghz/protocols/hormann_bisecur.c) with the encoder and the file serializers left
 * out - this app only ever reads, and never writes a .sub.
 */
#pragma once

#include <lib/subghz/protocols/base.h>

#define SUBGHZ_PROTOCOL_HORMANN_BISECUR_NAME "Hormann BiSecur"

extern const SubGhzProtocol ra_protocol_hormann_bisecur;

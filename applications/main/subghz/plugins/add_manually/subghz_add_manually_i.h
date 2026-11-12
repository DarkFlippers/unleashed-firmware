/**
 * @file subghz_add_manually_i.h
 * @brief Shared between the plugin's own scenes: the type menu's labels, and the steps that turn a
 *        chosen type and band into a generated protocol.
 */
#pragma once

#include <subghz/subghz_i.h>

/** The type's row in the menu, or NULL for a variant that has no row of its own. */
const char* subghz_set_type_name(SetType type);

/** Reset gen_info and fill it with freshly generated data for @p type. */
void subghz_scene_set_type_fill(SubGhz* subghz, SetType type);

/** Generate the protocol from gen_info, or ask for the first of its fields in Advanced.
 *  Returns whether the event was consumed; a failed generation puts up its own error screen. */
bool subghz_scene_set_type_proceed(SubGhz* subghz);

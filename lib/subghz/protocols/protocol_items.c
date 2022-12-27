#include "protocol_items.h"

const SubGhzProtocol* subghz_protocol_registry_items[] = {
    &subghz_protocol_gate_tx,       &subghz_protocol_keeloq,      &subghz_protocol_star_line,
    &subghz_protocol_nice_flo,      &subghz_protocol_came,        &subghz_protocol_faac_slh,
    &subghz_protocol_nice_flor_s,   &subghz_protocol_came_twee,   &subghz_protocol_came_atomo,
    &subghz_protocol_nero_sketch,   &subghz_protocol_ido,         &subghz_protocol_kia,
    &subghz_protocol_hormann,       &subghz_protocol_nero_radio,  &subghz_protocol_somfy_telis,
    &subghz_protocol_somfy_keytis,  &subghz_protocol_scher_khan,  &subghz_protocol_princeton,
    &subghz_protocol_raw,           &subghz_protocol_linear,      &subghz_protocol_secplus_v2,
    &subghz_protocol_secplus_v1,    &subghz_protocol_megacode,    &subghz_protocol_holtek,
    &subghz_protocol_chamb_code,    &subghz_protocol_power_smart, &subghz_protocol_marantec,
    &subghz_protocol_bett,          &subghz_protocol_doitrand,    &subghz_protocol_phoenix_v2,
    &subghz_protocol_honeywell_wdb, &subghz_protocol_magellan,    &subghz_protocol_intertechno_v3,
    &subghz_protocol_clemsa,        &subghz_protocol_ansonic,     &subghz_protocol_pocsag,
    &subghz_protocol_smc5326,
};

const SubGhzProtocolRegistry subghz_protocol_registry = {
    .items = subghz_protocol_registry_items,
    .size = COUNT_OF(subghz_protocol_registry_items)};
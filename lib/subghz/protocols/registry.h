#pragma once

#include "../types.h"

#include "princeton.h"
#include "keeloq.h"
#include "star_line.h"
#include "nice_flo.h"
#include "came.h"
#include "faac_slh.h"
#include "nice_flor_s.h"
#include "came_twee.h"
#include "came_atomo.h"
#include "nero_sketch.h"
#include "ido.h"
#include "kia.h"
#include "hormann.h"
#include "nero_radio.h"
#include "somfy_telis.h"
#include "somfy_keytis.h"
#include "scher_khan.h"
#include "gate_tx.h"
#include "raw.h"
#include "linear.h"
#include "secplus_v2.h"
#include "secplus_v1.h"
#include "megacode.h"
#include "holtek.h"
#include "chamberlain_code.h"
#include "power_smart.h"
#include "marantec.h"
#include "bett.h"
#include "doitrand.h"
#include "phoenix_v2.h"
#include "honeywell_wdb.h"
#include "magellen.h"
#include "intertechno_v3.h"
#include "clemsa.h"
#include "oregon2.h"

/**
 * Registration by name SubGhzProtocol.
 * @param name Protocol name
 * @return SubGhzProtocol* pointer to a SubGhzProtocol instance
 */
const SubGhzProtocol* subghz_protocol_registry_get_by_name(const char* name);

/**
 * Registration protocol by index in array SubGhzProtocol.
 * @param index Protocol by index in array
 * @return SubGhzProtocol* pointer to a SubGhzProtocol instance
 */
const SubGhzProtocol* subghz_protocol_registry_get_by_index(size_t index);

/**
 * Getting the number of registered protocols.
 * @return Number of protocols
 */
size_t subghz_protocol_registry_count();

#include "lfrfid_protocols.h"
#include "protocol_em4100.h"
#include "protocol_electra.h"
#include "protocol_h10301.h"
#include "protocol_idteck.h"
#include "protocol_indala26.h"
#include "protocol_io_prox_xsf.h"
#include "protocol_awid.h"
#include "protocol_fdx_a.h"
#include "protocol_fdx_b.h"
#include "protocol_hid_generic.h"
#include "protocol_hid_ex_generic.h"
#include "protocol_pyramid.h"
#include "protocol_viking.h"
#include "protocol_jablotron.h"
#include "protocol_paradox.h"
#include "protocol_pac_stanley.h"
#include "protocol_keri.h"
#include "protocol_gallagher.h"
#include "protocol_nexwatch.h"
#include "protocol_securakey.h"
#include "protocol_gproxii.h"

const ProtocolBase* lfrfid_protocols[] = {
    [LFRFIDProtocolEM4100] = &protocol_em4100,
    [LFRFIDProtocolEM410032] = &protocol_em4100_32,
    [LFRFIDProtocolEM410016] = &protocol_em4100_16,
    [LFRFIDProtocolElectra] = &protocol_electra,
    [LFRFIDProtocolH10301] = &protocol_h10301,
    [LFRFIDProtocolIdteck] = &protocol_idteck,
    [LFRFIDProtocolIndala26] = &protocol_indala26,
    [LFRFIDProtocolIOProxXSF] = &protocol_io_prox_xsf,
    [LFRFIDProtocolAwid] = &protocol_awid,
    [LFRFIDProtocolFDXA] = &protocol_fdx_a,
    [LFRFIDProtocolFDXB] = &protocol_fdx_b,
    [LFRFIDProtocolHidGeneric] = &protocol_hid_generic,
    [LFRFIDProtocolHidExGeneric] = &protocol_hid_ex_generic,
    [LFRFIDProtocolPyramid] = &protocol_pyramid,
    [LFRFIDProtocolViking] = &protocol_viking,
    [LFRFIDProtocolJablotron] = &protocol_jablotron,
    [LFRFIDProtocolParadox] = &protocol_paradox,
    [LFRFIDProtocolPACStanley] = &protocol_pac_stanley,
    [LFRFIDProtocolKeri] = &protocol_keri,
    [LFRFIDProtocolGallagher] = &protocol_gallagher,
    [LFRFIDProtocolNexwatch] = &protocol_nexwatch,
    [LFRFIDProtocolSecurakey] = &protocol_securakey,
    [LFRFIDProtocolGProxII] = &protocol_gproxii,
};

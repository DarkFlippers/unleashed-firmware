#pragma once
#include <toolbox/protocols/protocol.h>
#include "../tools/t5577.h"

typedef enum {
    LFRFIDFeatureASK = 1 << 0, /** ASK Demodulation */
    LFRFIDFeaturePSK = 1 << 1, /** PSK Demodulation */
} LFRFIDFeature;

typedef enum {
    LFRFIDProtocolEM4100,
    LFRFIDProtocolH10301,
    LFRFIDProtocolIndala26,
    LFRFIDProtocolIOProxXSF,
    LFRFIDProtocolAwid,
    LFRFIDProtocolFDXA,
    LFRFIDProtocolFDXB,
    LFRFIDProtocolHidGeneric,
    LFRFIDProtocolHidExGeneric,
    LFRFIDProtocolPyramid,
    LFRFIDProtocolViking,
    LFRFIDProtocolJablotron,
    LFRFIDProtocolParadox,
    LFRFIDProtocolPACStanley,
    LFRFIDProtocolKeri,
    LFRFIDProtocolGallagher,
    LFRFIDProtocolMax,
} LFRFIDProtocol;

extern const ProtocolBase* lfrfid_protocols[];

typedef enum {
    LFRFIDWriteTypeT5577,
} LFRFIDWriteType;

typedef struct {
    LFRFIDWriteType write_type;
    union {
        LFRFIDT5577 t5577;
    };
} LFRFIDWriteRequest;
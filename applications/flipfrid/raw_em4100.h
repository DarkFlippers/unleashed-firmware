#pragma once
#include <toolbox/protocols/protocol.h>


enum FlipFridProtocol {
    CLEAN,
    BAD_CRC,
};

extern const ProtocolBase protocol_raw_em4100;
extern const ProtocolBase protocol_raw_wrong_crc_em4100;
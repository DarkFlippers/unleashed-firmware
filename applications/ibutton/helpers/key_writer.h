#pragma once
#include "../ibutton_key.h"
#include <one_wire_master.h>

class KeyWriter {
public:
    enum class Error : uint8_t {
        OK,
        SAME_KEY,
        NO_DETECT,
        CANNOT_WRITE,
    };

    KeyWriter(OneWireMaster* onewire_master);
    ~KeyWriter();

    KeyWriter::Error write(iButtonKey* key);
    void start();
    void stop();

private:
    OneWireMaster* onewire_master;

    KeyWriter::Error write_internal(iButtonKey* key);
    bool compare_key_ds1990(iButtonKey* key);

    // write strategy
    bool write_1990_1(iButtonKey* key);
    bool write_1990_2(iButtonKey* key);
    bool write_TM2004(iButtonKey* key);
    bool write_TM01(iButtonKey* key);

    void onewire_write_one_bit(bool value, uint32_t delay = 10000);
    void write_byte_ds1990(uint8_t data);
};

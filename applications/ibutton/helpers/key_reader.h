#pragma once
#include <stdint.h>
#include <furi.h>
#include "cyfral_decoder.h"
#pragma once
#include "metakom_decoder.h"
#include "../ibutton_key.h"
#include <one_wire_master.h>
#include <one_wire_slave.h>

class KeyReader {
public:
    enum class Error : uint8_t {
        EMPTY,
        CRC_ERROR,
        NOT_ARE_KEY,
        OK,
    };

    void start();
    void stop();
    KeyReader::Error read(iButtonKey* key);
    KeyReader(OneWireMaster* onewire_master);
    ~KeyReader();

private:
    bool read_key(iButtonKeyType* key_type, uint8_t* data, uint8_t data_size);
    bool verify_key(iButtonKeyType key_type, const uint8_t* const data, uint8_t data_size);

    // cyfral and metakom readers data
    void comparator_trigger_callback(void* hcomp, void* comp_ctx);
    void (*comparator_callback_pointer)(void* hcomp, void* comp_ctx);

    void start_comaparator(void);
    void stop_comaparator(void);
    uint32_t last_dwt_value;

    CyfralDecoder cyfral_decoder;
    MetakomDecoder metakom_decoder;

    // mode
    uint32_t read_mode_switch_time;
    enum class ReadMode : uint8_t {
        CYFRAL_METAKOM,
        DALLAS,
    };
    ReadMode read_mode;

    // one wire
    OneWireMaster* onewire_master;

    void switch_to(ReadMode mode);
    void switch_mode_if_needed();
};
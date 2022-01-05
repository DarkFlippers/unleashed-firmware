#pragma once
#include "pulse_sequencer.h"
#include "../ibutton_key.h"
#include <one_wire_slave.h>
#include <one_wire_device_ds_1990.h>
#include <atomic>

class KeyEmulator {
public:
    KeyEmulator(OneWireSlave* onewire_slave);
    ~KeyEmulator();

    void start(iButtonKey* key);
    bool emulated();
    void stop();

private:
    DS1990 dallas_key;
    OneWireSlave* onewire_slave;

    PulseSequencer pulser;
    uint32_t pulse_data[72];

    std::atomic<bool> anything_emulated;

    void start_cyfral_emulate(iButtonKey* key);
    void start_metakom_emulate(iButtonKey* key);
    void start_dallas_emulate(iButtonKey* key);

    void set_pulse_data_cyfral(uint8_t index, const uint32_t* data);
    void set_pulse_data_metakom(uint8_t index, const uint32_t* data);

    void result_callback(bool success, void* ctx);
};

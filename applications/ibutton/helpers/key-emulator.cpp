#include "key-emulator.h"
#include <callback-connector.h>

KeyEmulator::~KeyEmulator() {
    stop();
}

KeyEmulator::KeyEmulator(OneWireSlave* _onewire_slave)
    : dallas_key{0, 0, 0, 0, 0, 0, 0} {
    onewire_slave = _onewire_slave;

    auto cb = cbc::obtain_connector(this, &KeyEmulator::result_callback);
    onewire_slave->set_result_callback(cb, this);
}

void KeyEmulator::start(iButtonKey* key) {
    anything_emulated = false;
    stop();

    // pulldown pull pin, to prevent low-pass filtering by the RFID part of the schematic
    furi_hal_rfid_pin_pull_pulldown();

    switch(key->get_key_type()) {
    case iButtonKeyType::KeyDallas:
        start_dallas_emulate(key);
        break;
    case iButtonKeyType::KeyCyfral:
        start_cyfral_emulate(key);
        break;
    case iButtonKeyType::KeyMetakom:
        start_metakom_emulate(key);
        break;
    }
}

bool KeyEmulator::emulated() {
    bool result = false;

    if(anything_emulated) {
        anything_emulated = false;
        result = true;
    }

    return result;
}

void KeyEmulator::stop() {
    onewire_slave->stop();
    pulser.stop();
    furi_hal_rfid_pins_reset();
}

void KeyEmulator::start_cyfral_emulate(iButtonKey* key) {
    furi_assert(key->get_key_type() == iButtonKeyType::KeyCyfral);
    furi_assert(key->get_type_data_size() == 2);

    const uint32_t cyfral_period_full = 8000;
    const uint32_t cyfral_period_one[2] = {
        uint32_t(cyfral_period_full * 0.33f), uint32_t(cyfral_period_full * 0.66f)};
    const uint32_t cyfral_period_zero[2] = {
        uint32_t(cyfral_period_full * 0.66f), uint32_t(cyfral_period_full * 0.33f)};
    uint8_t pd_index = 0;
    uint8_t* key_data = key->get_data();

    // start nibble
    set_pulse_data_cyfral(pd_index, cyfral_period_zero);
    pd_index++;
    set_pulse_data_cyfral(pd_index, cyfral_period_zero);
    pd_index++;
    set_pulse_data_cyfral(pd_index, cyfral_period_zero);
    pd_index++;
    set_pulse_data_cyfral(pd_index, cyfral_period_one);
    pd_index++;

    // data nibbles x 8
    for(int8_t i = key->get_type_data_size() - 1; i >= 0; i--) {
        for(int8_t j = 3; j >= 0; j--) {
            switch((key_data[i] >> (j * 2)) & 0b00000011) {
            case 0b11:
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_zero);
                pd_index++;
                break;
            case 0b10:
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_zero);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                break;
            case 0b01:
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_zero);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                break;
            case 0b00:
                set_pulse_data_cyfral(pd_index, cyfral_period_zero);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                set_pulse_data_cyfral(pd_index, cyfral_period_one);
                pd_index++;
                break;
            default:
                // cannot be anyway
                furi_check(false);
                break;
            }
        }
    }

    // 4 (nibbles) x (8 data + 1 start) = 4 x 9 = 36
    if(pd_index != 36) {
        // something is very wrong
        furi_check(false);
    }

    pulser.set_periods(pulse_data, 72, false);
    pulser.start();
}

void KeyEmulator::start_metakom_emulate(iButtonKey* key) {
    furi_assert(key->get_key_type() == iButtonKeyType::KeyMetakom);
    furi_assert(key->get_type_data_size() == 4);

    const uint32_t metakom_period_full = 8000;
    const uint32_t metakom_period_zero[2] = {
        uint32_t(metakom_period_full * 0.33f), uint32_t(metakom_period_full * 0.66f)};
    const uint32_t metakom_period_one[2] = {
        uint32_t(metakom_period_full * 0.66f), uint32_t(metakom_period_full * 0.33f)};
    uint8_t pd_index = 0;

    uint8_t* key_data = key->get_data();

    // start pulse
    pulse_data[0] = metakom_period_full;

    // start triplet
    set_pulse_data_metakom(pd_index, metakom_period_zero);
    pd_index++;
    set_pulse_data_metakom(pd_index, metakom_period_one);
    pd_index++;
    set_pulse_data_metakom(pd_index, metakom_period_zero);
    pd_index++;

    for(int8_t i = key->get_type_data_size() - 1; i >= 0; i--) {
        for(int8_t j = 7; j >= 0; j--) {
            if(((key_data[i] >> j) & 0b00000001) == 1) {
                set_pulse_data_metakom(pd_index, metakom_period_one);
                pd_index++;
            } else {
                set_pulse_data_metakom(pd_index, metakom_period_zero);
                pd_index++;
            }
        }
    }

    // 4 byte x 8 bits + 3 start bits = 35
    if(pd_index != 35) {
        // something is very wrong
        furi_check(false);
    }

    pulser.set_periods(pulse_data, 71, false);
    pulser.start();
}

void KeyEmulator::start_dallas_emulate(iButtonKey* key) {
    furi_assert(key->get_key_type() == iButtonKeyType::KeyDallas);
    furi_assert(key->get_type_data_size() == 8);

    onewire_slave->deattach();
    memcpy(dallas_key.id_storage, key->get_data(), key->get_type_data_size());
    onewire_slave->attach(&dallas_key);
    onewire_slave->start();
}

void KeyEmulator::set_pulse_data_cyfral(uint8_t index, const uint32_t* data) {
    pulse_data[index * 2] = data[0];
    pulse_data[index * 2 + 1] = data[1];
}

void KeyEmulator::set_pulse_data_metakom(uint8_t index, const uint32_t* data) {
    // damn start pulse
    pulse_data[(index * 2) + 1] = data[0];
    pulse_data[(index * 2) + 2] = data[1];
}

void KeyEmulator::result_callback(bool success, void* ctx) {
    KeyEmulator* _this = static_cast<KeyEmulator*>(ctx);

    _this->anything_emulated = true;
}
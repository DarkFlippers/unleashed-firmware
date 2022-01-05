#include "key_reader.h"
#include "key_commands.h"
#include <callback-connector.h>
#include <maxim_crc.h>

extern COMP_HandleTypeDef hcomp1;

KeyReader::Error KeyReader::read(iButtonKey* key) {
    uint8_t tmp_key_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    iButtonKeyType key_type;

    KeyReader::Error result = KeyReader::Error::EMPTY;

    if(read_key(&key_type, tmp_key_data, 8)) {
        switch(key_type) {
        case iButtonKeyType::KeyDallas:
            if(verify_key(key_type, tmp_key_data, 8)) {
                if(maxim_crc8(tmp_key_data, 8) == 0) {
                    if(tmp_key_data[0] == 0x01) {
                        result = KeyReader::Error::OK;
                    } else {
                        result = KeyReader::Error::NOT_ARE_KEY;
                    }
                } else {
                    result = KeyReader::Error::CRC_ERROR;
                }
            }

            break;
        case iButtonKeyType::KeyCyfral:
            result = KeyReader::Error::OK;
            break;
        case iButtonKeyType::KeyMetakom:
            result = KeyReader::Error::OK;
            break;
        }

        if(result != KeyReader::Error::EMPTY) {
            key->set_type(key_type);
            key->set_data(tmp_key_data, 8);
        }
    }

    switch_mode_if_needed();

    return result;
}

KeyReader::KeyReader(OneWireMaster* _onewire_master) {
    onewire_master = _onewire_master;
    read_mode_switch_time = 0;
    read_mode = ReadMode::DALLAS;
}

KeyReader::~KeyReader() {
    stop();
}

bool KeyReader::read_key(iButtonKeyType* key_type, uint8_t* data, uint8_t data_size) {
    bool readed = false;

    switch(read_mode) {
    case ReadMode::DALLAS:
        __disable_irq();
        if(onewire_master->search(data)) {
            onewire_master->reset_search();
            readed = true;
            *key_type = iButtonKeyType::KeyDallas;
        } else {
            onewire_master->reset_search();
        }
        __enable_irq();
        break;
    case ReadMode::CYFRAL_METAKOM:
        if(cyfral_decoder.read(data, 2)) {
            readed = true;
            *key_type = iButtonKeyType::KeyCyfral;
        } else if(metakom_decoder.read(data, 4)) {
            readed = true;
            *key_type = iButtonKeyType::KeyMetakom;
        }
        break;
    }

    return readed;
}

bool KeyReader::verify_key(iButtonKeyType key_type, const uint8_t* const data, uint8_t data_size) {
    bool result = true;

    switch(key_type) {
    case iButtonKeyType::KeyDallas:
        switch_to(ReadMode::DALLAS);
        __disable_irq();
        if(onewire_master->reset()) {
            onewire_master->write(DS1990::CMD_READ_ROM);
            for(uint8_t i = 0; i < data_size; i++) {
                if(onewire_master->read() != data[i]) {
                    result = false;
                }
            }
        } else {
            result = false;
            break;
        }
        __enable_irq();
        break;

    default:
        result = false;
        break;
    }

    return result;
}

void KeyReader::start_comaparator(void) {
    furi_hal_rfid_pins_reset();

    // pulldown pull pin, we sense the signal through the analog part of the RFID schematic
    furi_hal_rfid_pin_pull_pulldown();

    comparator_callback_pointer =
        cbc::obtain_connector(this, &KeyReader::comparator_trigger_callback);
    api_interrupt_add(comparator_callback_pointer, InterruptTypeComparatorTrigger, this);
    last_dwt_value = DWT->CYCCNT;
    HAL_COMP_Start(&hcomp1);
}

void KeyReader::stop_comaparator(void) {
    furi_hal_rfid_pins_reset();

    // rfid_pins_reset will disable ibutton pin
    furi_hal_ibutton_start();

    HAL_COMP_Stop(&hcomp1);
    api_interrupt_remove(comparator_callback_pointer, InterruptTypeComparatorTrigger);
}

void KeyReader::comparator_trigger_callback(void* hcomp, void* comp_ctx) {
    KeyReader* _this = static_cast<KeyReader*>(comp_ctx);

    if(hcomp == &hcomp1) {
        uint32_t current_dwt_value = DWT->CYCCNT;

        _this->cyfral_decoder.process_front(
            hal_gpio_get_rfid_in_level(), current_dwt_value - last_dwt_value);
        _this->metakom_decoder.process_front(
            hal_gpio_get_rfid_in_level(), current_dwt_value - last_dwt_value);

        last_dwt_value = current_dwt_value;
    }
}

void KeyReader::switch_to(ReadMode mode) {
    switch(mode) {
    case ReadMode::DALLAS:
        onewire_master->start();
        stop_comaparator();
        break;
    case ReadMode::CYFRAL_METAKOM:
        onewire_master->stop();
        start_comaparator();
        break;
    }

    read_mode = mode;
}

void KeyReader::switch_mode_if_needed() {
    if(osKernelGetTickCount() - read_mode_switch_time > (osKernelGetTickFreq() / 5)) {
        read_mode_switch_time = osKernelGetTickCount();
        switch(read_mode) {
        case ReadMode::DALLAS:
            switch_to(ReadMode::CYFRAL_METAKOM);
            break;
        case ReadMode::CYFRAL_METAKOM:
            switch_to(ReadMode::DALLAS);
            break;
        }
    }
}

void KeyReader::start() {
    furi_hal_power_enable_otg();
    switch_to(ReadMode::CYFRAL_METAKOM);
}

void KeyReader::stop() {
    furi_hal_power_disable_otg();
    onewire_master->stop();
    stop_comaparator();
}

#include "key-writer.h"
#include "key-commands.h"

KeyWriter::KeyWriter(OneWireMaster* _onewire_master) {
    onewire_master = _onewire_master;
}

KeyWriter::~KeyWriter() {
    stop();
}

KeyWriter::Error KeyWriter::write(iButtonKey* key) {
    return write_internal(key);
}

void KeyWriter::start() {
    api_hal_power_enable_otg();
    onewire_master->start();
}

void KeyWriter::stop() {
    api_hal_power_disable_otg();
    onewire_master->stop();
}

KeyWriter::Error KeyWriter::write_internal(iButtonKey* key) {
    Error result = Error::NO_DETECT;
    bool same_key = false;

    osKernelLock();
    bool presence = onewire_master->reset();
    osKernelUnlock();

    if(presence) {
        switch(key->get_key_type()) {
        case iButtonKeyType::KeyDallas:
            same_key = compare_key_ds1990(key);

            if(!same_key) {
                bool write_result = false;
                // currently we can write:
                // RW1990, TM08v2, TM08vi-2 by write_1990_1()
                // RW2004, RW2004 with EEPROM by write_TM2004();

                if(!write_result) {
                    write_result = write_1990_1(key);
                }
                if(!write_result) {
                    write_result = write_1990_2(key);
                }
                if(!write_result) {
                    write_result = write_TM2004(key);
                }

                if(write_result) {
                    result = Error::OK;
                } else {
                    result = Error::CANNOT_WRITE;
                }
            } else {
                result = Error::SAME_KEY;
            }
            break;

        default:
            break;
        }
    }

    return result;
}

bool KeyWriter::compare_key_ds1990(iButtonKey* key) {
    bool result = false;

    if(key->get_key_type() == iButtonKeyType::KeyDallas) {
        __disable_irq();
        bool presence = onewire_master->reset();

        if(presence) {
            onewire_master->write(DS1990::CMD_READ_ROM);

            result = true;
            for(uint8_t i = 0; i < key->get_type_data_size(); i++) {
                if(key->get_data()[i] != onewire_master->read()) {
                    result = false;
                    break;
                }
            }
        }

        __enable_irq();
    }

    return result;
}

bool KeyWriter::write_1990_1(iButtonKey* key) {
    bool result = false;

    if(key->get_key_type() == iButtonKeyType::KeyDallas) {
        __disable_irq();

        // unlock
        onewire_master->reset();
        onewire_master->write(RW1990_1::CMD_WRITE_RECORD_FLAG);
        delay_us(10);
        onewire_write_one_bit(0, 5000);

        // write key
        onewire_master->reset();
        onewire_master->write(RW1990_1::CMD_WRITE_ROM);
        for(uint8_t i = 0; i < key->get_type_data_size(); i++) {
            // inverted key for RW1990.1
            write_byte_ds1990(~key->get_data()[i]);
            delay_us(30000);
        }

        // lock
        onewire_master->write(RW1990_1::CMD_WRITE_RECORD_FLAG);
        onewire_write_one_bit(1);

        __enable_irq();

        if(compare_key_ds1990(key)) {
            result = true;
        }
    }

    return result;
}

bool KeyWriter::write_1990_2(iButtonKey* key) {
    bool result = false;

    if(key->get_key_type() == iButtonKeyType::KeyDallas) {
        __disable_irq();

        // unlock
        onewire_master->reset();
        onewire_master->write(RW1990_2::CMD_WRITE_RECORD_FLAG);
        delay_us(10);
        onewire_write_one_bit(1, 5000);

        // write key
        onewire_master->reset();
        onewire_master->write(RW1990_2::CMD_WRITE_ROM);
        for(uint8_t i = 0; i < key->get_type_data_size(); i++) {
            write_byte_ds1990(key->get_data()[i]);
            delay_us(30000);
        }

        // lock
        onewire_master->write(RW1990_2::CMD_WRITE_RECORD_FLAG);
        onewire_write_one_bit(0);

        __enable_irq();

        if(compare_key_ds1990(key)) {
            result = true;
        }
    }

    return result;
}

bool KeyWriter::write_TM2004(iButtonKey* key) {
    uint8_t answer;
    bool result = true;

    if(key->get_key_type() == iButtonKeyType::KeyDallas) {
        __disable_irq();

        // write rom, addr is 0x0000
        onewire_master->reset();
        onewire_master->write(TM2004::CMD_WRITE_ROM);
        onewire_master->write(0x00);
        onewire_master->write(0x00);

        // write key
        for(uint8_t i = 0; i < key->get_type_data_size(); i++) {
            // write key byte
            onewire_master->write(key->get_data()[i]);
            answer = onewire_master->read();
            // TODO: check answer CRC

            // pulse indicating that data is correct
            delay_us(600);
            onewire_write_one_bit(1, 50000);

            // read writed key byte
            answer = onewire_master->read();

            // check that writed and readed are same
            if(key->get_data()[i] != answer) {
                result = false;
                break;
            }
        }

        if(!compare_key_ds1990(key)) {
            result = false;
        }

        onewire_master->reset();

        __enable_irq();
    } else {
        result = false;
    }

    return result;
}

bool KeyWriter::write_TM01(iButtonKey* key) {
    /*bool result = true;

    // TODO test and encoding
    __disable_irq();

    // unlock
    onewire_master->reset();
    onewire_master->write(TM01::CMD_WRITE_RECORD_FLAG);
    onewire_write_one_bit(1, 10000);

    // write key
    onewire_master->reset();
    onewire_master->write(TM01::CMD_WRITE_ROM);

    // TODO: key types
    //if(type == KEY_METAKOM || type == KEY_CYFRAL) {
    //} else {
    for(uint8_t i = 0; i < key->get_type_data_size(); i++) {
        write_byte_ds1990(key->get_data()[i]);
        delay_us(10000);
    }
    //}

    // lock
    onewire_master->write(TM01::CMD_WRITE_RECORD_FLAG);
    onewire_write_one_bit(0, 10000);

    __enable_irq();

    if(!compare_key_ds1990(key)) {
        result = false;
    }

    __disable_irq();

    if(key->get_key_type() == iButtonKeyType::KeyMetakom ||
       key->get_key_type() == iButtonKeyType::KeyCyfral) {
        onewire_master->reset();
        if(key->get_key_type() == iButtonKeyType::KeyCyfral)
            onewire_master->write(TM01::CMD_SWITCH_TO_CYFRAL);
        else
            onewire_master->write(TM01::CMD_SWITCH_TO_METAKOM);
        onewire_write_one_bit(1);
    }

    __enable_irq();

    return result;*/
    return false;
}

void KeyWriter::onewire_write_one_bit(bool value, uint32_t delay) {
    onewire_master->write_bit(value);
    delay_us(delay);
}

void KeyWriter::write_byte_ds1990(uint8_t data) {
    for(uint8_t n_bit = 0; n_bit < 8; n_bit++) {
        onewire_master->write_bit(data & 1);
        delay_us(5000);
        data = data >> 1;
    }
}
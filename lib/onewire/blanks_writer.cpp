#include "blanks_writer.h"

class RW1990_1 {
public:
    constexpr static const uint8_t CMD_WRITE_RECORD_FLAG = 0xD1;
    constexpr static const uint8_t CMD_READ_RECORD_FLAG = 0xB5;
    constexpr static const uint8_t CMD_WRITE_ROM = 0xD5;
};

class RW1990_2 {
public:
    constexpr static const uint8_t CMD_WRITE_RECORD_FLAG = 0x1D;
    constexpr static const uint8_t CMD_READ_RECORD_FLAG = 0x1E;
    constexpr static const uint8_t CMD_WRITE_ROM = 0xD5;
};

class TM2004 {
public:
    constexpr static const uint8_t CMD_READ_STATUS = 0xAA;
    constexpr static const uint8_t CMD_READ_MEMORY = 0xF0;
    constexpr static const uint8_t CMD_WRITE_ROM = 0x3C;
    constexpr static const uint8_t CMD_FINALIZATION = 0x35;

    constexpr static const uint8_t ANSWER_READ_MEMORY = 0xF5;
};

class TM01 {
public:
    constexpr static const uint8_t CMD_WRITE_RECORD_FLAG = 0xC1;
    constexpr static const uint8_t CMD_WRITE_ROM = 0xC5;
    constexpr static const uint8_t CMD_SWITCH_TO_CYFRAL = 0xCA;
    constexpr static const uint8_t CMD_SWITCH_TO_METAKOM = 0xCB;
};

class DS1990 {
public:
    constexpr static const uint8_t CMD_READ_ROM = 0x33;
};

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <furi-hal.h>

void BlanksWriter::onewire_release(void) {
    hal_gpio_write(gpio, true);
}

void BlanksWriter::onewire_write_one_bit(bool value, uint32_t delay = 10000) {
    onewire->write_bit(value);
    delay_us(delay);
    onewire_release();
}

BlanksWriter::BlanksWriter(const GpioPin* one_wire_gpio) {
    gpio = one_wire_gpio;
    onewire = new OneWireMaster(gpio);
}

BlanksWriter::~BlanksWriter() {
    free(onewire);
}

WriterResult BlanksWriter::write(KeyType type, const uint8_t* key, uint8_t key_length) {
    uint8_t write_result = -1;
    WriterResult result = WR_ERROR;

    bool same_key = false;

    osKernelLock();
    bool presence = onewire->reset();
    osKernelUnlock();

    if(presence) {
        switch(type) {
        case KeyType::KEY_DS1990:
            same_key = compare_key_ds1990(key, key_length);

            if(!same_key) {
                // currently we can write:
                // RW1990, TM08v2, TM08vi-2 by write_1990_1()
                // RW2004, RW2004 with EEPROM by write_TM2004();

                if(write_result != 1) {
                    write_result = write_1990_1(key, key_length);
                }
                if(write_result != 1) {
                    write_result = write_1990_2(key, key_length);
                }
                if(write_result != 1) {
                    write_result = write_TM2004(key, key_length);
                }

                if(write_result == 1) {
                    result = WR_OK;
                } else if(write_result == 0) {
                    result = WR_ERROR;
                }
            } else {
                write_result = 0;
                result = WR_SAME_KEY;
            }
            break;

        default:
            break;
        }
    }

    return result;
}

bool BlanksWriter::write_TM2004(const uint8_t* key, uint8_t key_length) {
    uint8_t answer;
    bool result = true;

    osKernelLock();
    __disable_irq();

    // write rom, addr is 0x0000
    onewire->reset();
    onewire->write(TM2004::CMD_WRITE_ROM);
    onewire->write(0x00);
    onewire->write(0x00);

    // write key
    for(uint8_t i = 0; i < key_length; i++) {
        // write key byte
        onewire->write(key[i]);
        answer = onewire->read();
        // TODO: check answer CRC

        // pulse indicating that data is correct
        delay_us(600);
        onewire_write_one_bit(1, 50000);

        // read writed key byte
        answer = onewire->read();

        // check that writed and readed are same
        if(key[i] != answer) {
            result = false;
            break;
        }
    }

    onewire->reset();

    __enable_irq();
    osKernelUnlock();

    return result;
}

bool BlanksWriter::write_1990_1(const uint8_t* key, uint8_t key_length) {
    bool result = true;

    osKernelLock();
    __disable_irq();

    // unlock
    onewire->reset();
    onewire->write(RW1990_1::CMD_WRITE_RECORD_FLAG);
    delay_us(10);
    onewire_write_one_bit(0, 5000);

    // write key
    onewire->reset();
    onewire->write(RW1990_1::CMD_WRITE_ROM);
    for(uint8_t i = 0; i < key_length; i++) {
        // inverted key for RW1990.1
        write_byte_ds1990(~key[i]);
        delay_us(30000);
    }

    // lock
    onewire->write(RW1990_1::CMD_WRITE_RECORD_FLAG);
    onewire_write_one_bit(1);

    __enable_irq();
    osKernelUnlock();

    if(!compare_key_ds1990(key, key_length)) {
        result = false;
    }

    return result;
}

bool BlanksWriter::write_1990_2(const uint8_t* key, uint8_t key_length) {
    bool result = true;

    osKernelLock();
    __disable_irq();

    // unlock
    onewire->reset();
    onewire->write(RW1990_2::CMD_WRITE_RECORD_FLAG);
    delay_us(10);
    onewire_write_one_bit(1, 5000);

    // write key
    onewire->reset();
    onewire->write(RW1990_2::CMD_WRITE_ROM);
    for(uint8_t i = 0; i < key_length; i++) {
        write_byte_ds1990(key[i]);
        delay_us(30000);
    }

    // lock
    onewire->write(RW1990_2::CMD_WRITE_RECORD_FLAG);
    onewire_write_one_bit(0);

    __enable_irq();
    osKernelUnlock();

    if(!compare_key_ds1990(key, key_length)) {
        result = false;
    }

    return result;
}

// TODO: untested
bool BlanksWriter::write_TM01(KeyType type, const uint8_t* key, uint8_t key_length) {
    bool result = true;

    osKernelLock();
    __disable_irq();

    // unlock
    onewire->reset();
    onewire->write(TM01::CMD_WRITE_RECORD_FLAG);
    onewire_write_one_bit(1, 10000);

    // write key
    onewire->reset();
    onewire->write(TM01::CMD_WRITE_ROM);

    // TODO: key types
    //if(type == KEY_METAKOM || type == KEY_CYFRAL) {
    //} else {
    for(uint8_t i = 0; i < key_length; i++) {
        write_byte_ds1990(key[i]);
        delay_us(10000);
    }
    //}

    // lock
    onewire->write(TM01::CMD_WRITE_RECORD_FLAG);
    onewire_write_one_bit(0, 10000);

    __enable_irq();
    osKernelUnlock();

    if(!compare_key_ds1990(key, key_length)) {
        result = false;
    }

    osKernelLock();
    __disable_irq();

    if(type == KEY_METAKOM || type == KEY_CYFRAL) {
        onewire->reset();
        if(type == KEY_CYFRAL)
            onewire->write(TM01::CMD_SWITCH_TO_CYFRAL);
        else
            onewire->write(TM01::CMD_SWITCH_TO_METAKOM);
        onewire_write_one_bit(1);
    }

    __enable_irq();
    osKernelUnlock();

    return result;
}

void BlanksWriter::write_byte_ds1990(uint8_t data) {
    for(uint8_t n_bit = 0; n_bit < 8; n_bit++) {
        onewire->write_bit(data & 1);
        onewire_release();
        delay_us(5000);
        data = data >> 1;
    }
}

bool BlanksWriter::compare_key_ds1990(const uint8_t* key, uint8_t key_length) {
    uint8_t buff[key_length];
    bool result = false;

    osKernelLock();
    bool presence = onewire->reset();
    osKernelUnlock();

    if(presence) {
        osKernelLock();
        __disable_irq();
        onewire->write(DS1990::CMD_READ_ROM);
        onewire->read_bytes(buff, key_length);
        __enable_irq();
        osKernelUnlock();

        result = true;
        for(uint8_t i = 0; i < 8; i++) {
            if(key[i] != buff[i]) {
                result = false;
                break;
            }
        }
    }
    return result;
}

void BlanksWriter::start() {
    onewire->start();
}

void BlanksWriter::stop() {
    onewire->stop();
}
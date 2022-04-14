#include <furi.h>
#include <furi_hal.h>
#include "ibutton_writer.h"
#include "ibutton_key_command.h"

/*********************** PRIVATE ***********************/

struct iButtonWriter {
    OneWireHost* host;
};

static void writer_write_one_bit(iButtonWriter* writer, bool value, uint32_t delay) {
    onewire_host_write_bit(writer->host, value);
    furi_hal_delay_us(delay);
}

static void writer_write_byte_ds1990(iButtonWriter* writer, uint8_t data) {
    for(uint8_t n_bit = 0; n_bit < 8; n_bit++) {
        onewire_host_write_bit(writer->host, data & 1);
        furi_hal_delay_us(5000);
        data = data >> 1;
    }
}

static bool writer_compare_key_ds1990(iButtonWriter* writer, iButtonKey* key) {
    bool result = false;

    if(ibutton_key_get_type(key) == iButtonKeyDS1990) {
        FURI_CRITICAL_ENTER();
        bool presence = onewire_host_reset(writer->host);

        if(presence) {
            onewire_host_write(writer->host, DS1990_CMD_READ_ROM);

            result = true;
            for(uint8_t i = 0; i < ibutton_key_get_data_size(key); i++) {
                if(ibutton_key_get_data_p(key)[i] != onewire_host_read(writer->host)) {
                    result = false;
                    break;
                }
            }
        }

        FURI_CRITICAL_EXIT();
    }

    return result;
}

static bool writer_write_TM2004(iButtonWriter* writer, iButtonKey* key) {
    uint8_t answer;
    bool result = true;

    if(ibutton_key_get_type(key) == iButtonKeyDS1990) {
        FURI_CRITICAL_ENTER();

        // write rom, addr is 0x0000
        onewire_host_reset(writer->host);
        onewire_host_write(writer->host, TM2004_CMD_WRITE_ROM);
        onewire_host_write(writer->host, 0x00);
        onewire_host_write(writer->host, 0x00);

        // write key
        for(uint8_t i = 0; i < ibutton_key_get_data_size(key); i++) {
            // write key byte
            onewire_host_write(writer->host, ibutton_key_get_data_p(key)[i]);
            answer = onewire_host_read(writer->host);
            // TODO: check answer CRC

            // pulse indicating that data is correct
            furi_hal_delay_us(600);
            writer_write_one_bit(writer, 1, 50000);

            // read written key byte
            answer = onewire_host_read(writer->host);

            // check that written and read are same
            if(ibutton_key_get_data_p(key)[i] != answer) {
                result = false;
                break;
            }
        }

        if(!writer_compare_key_ds1990(writer, key)) {
            result = false;
        }

        onewire_host_reset(writer->host);

        FURI_CRITICAL_EXIT();
    } else {
        result = false;
    }

    return result;
}

static bool writer_write_1990_1(iButtonWriter* writer, iButtonKey* key) {
    bool result = false;

    if(ibutton_key_get_type(key) == iButtonKeyDS1990) {
        FURI_CRITICAL_ENTER();

        // unlock
        onewire_host_reset(writer->host);
        onewire_host_write(writer->host, RW1990_1_CMD_WRITE_RECORD_FLAG);
        furi_hal_delay_us(10);
        writer_write_one_bit(writer, 0, 5000);

        // write key
        onewire_host_reset(writer->host);
        onewire_host_write(writer->host, RW1990_1_CMD_WRITE_ROM);
        for(uint8_t i = 0; i < ibutton_key_get_data_size(key); i++) {
            // inverted key for RW1990.1
            writer_write_byte_ds1990(writer, ~ibutton_key_get_data_p(key)[i]);
            furi_hal_delay_us(30000);
        }

        // lock
        onewire_host_write(writer->host, RW1990_1_CMD_WRITE_RECORD_FLAG);
        writer_write_one_bit(writer, 1, 10000);

        FURI_CRITICAL_EXIT();

        if(writer_compare_key_ds1990(writer, key)) {
            result = true;
        }
    }

    return result;
}

static bool writer_write_1990_2(iButtonWriter* writer, iButtonKey* key) {
    bool result = false;

    if(ibutton_key_get_type(key) == iButtonKeyDS1990) {
        FURI_CRITICAL_ENTER();

        // unlock
        onewire_host_reset(writer->host);
        onewire_host_write(writer->host, RW1990_2_CMD_WRITE_RECORD_FLAG);
        furi_hal_delay_us(10);
        writer_write_one_bit(writer, 1, 5000);

        // write key
        onewire_host_reset(writer->host);
        onewire_host_write(writer->host, RW1990_2_CMD_WRITE_ROM);
        for(uint8_t i = 0; i < ibutton_key_get_data_size(key); i++) {
            writer_write_byte_ds1990(writer, ibutton_key_get_data_p(key)[i]);
            furi_hal_delay_us(30000);
        }

        // lock
        onewire_host_write(writer->host, RW1990_2_CMD_WRITE_RECORD_FLAG);
        writer_write_one_bit(writer, 0, 10000);

        FURI_CRITICAL_EXIT();

        if(writer_compare_key_ds1990(writer, key)) {
            result = true;
        }
    }

    return result;
}

/*
// TODO: adapt and test
static bool writer_write_TM01(
    iButtonWriter* writer,
    iButtonKey type,
    const uint8_t* key,
    uint8_t key_length) {
    bool result = true;

    {
        // TODO test and encoding
        FURI_CRITICAL_ENTER();

        // unlock
        onewire_host_reset(writer->host);
        onewire_host_write(writer->host, TM01::CMD_WRITE_RECORD_FLAG);
        onewire_write_one_bit(1, 10000);

        // write key
        onewire_host_reset(writer->host);
        onewire_host_write(writer->host, TM01::CMD_WRITE_ROM);

        // TODO: key types
        //if(type == KEY_METAKOM || type == KEY_CYFRAL) {
        //} else {
        for(uint8_t i = 0; i < key->get_type_data_size(); i++) {
            write_byte_ds1990(key->get_data()[i]);
            furi_hal_delay_us(10000);
        }
        //}

        // lock
        onewire_host_write(writer->host, TM01::CMD_WRITE_RECORD_FLAG);
        onewire_write_one_bit(0, 10000);

        FURI_CRITICAL_EXIT();
    }

    if(!compare_key_ds1990(key)) {
        result = false;
    }

    {
        FURI_CRITICAL_ENTER();

        if(key->get_key_type() == iButtonKeyType::KeyMetakom ||
           key->get_key_type() == iButtonKeyType::KeyCyfral) {
            onewire_host_reset(writer->host);
            if(key->get_key_type() == iButtonKeyType::KeyCyfral)
                onewire_host_write(writer->host, TM01::CMD_SWITCH_TO_CYFRAL);
            else
                onewire_host_write(writer->host, TM01::CMD_SWITCH_TO_METAKOM);
            onewire_write_one_bit(1);
        }

        FURI_CRITICAL_EXIT();
    }

    return result;
}
*/

static iButtonWriterResult writer_write_DS1990(iButtonWriter* writer, iButtonKey* key) {
    iButtonWriterResult result = iButtonWriterNoDetect;
    bool same_key = writer_compare_key_ds1990(writer, key);

    if(!same_key) {
        // currently we can write:
        // RW1990_1, TM08v2, TM08vi-2 by write_1990_1()
        // RW1990_2 by write_1990_2()
        // RW2004, RW2004, TM2004 with EEPROM by write_TM2004();

        bool write_result = true;
        do {
            if(writer_write_1990_1(writer, key)) break;
            if(writer_write_1990_2(writer, key)) break;
            if(writer_write_TM2004(writer, key)) break;
            write_result = false;
        } while(false);

        if(write_result) {
            result = iButtonWriterOK;
        } else {
            result = iButtonWriterCannotWrite;
        }
    } else {
        result = iButtonWriterSameKey;
    }

    return result;
}

/*********************** PUBLIC ***********************/

iButtonWriter* ibutton_writer_alloc(OneWireHost* host) {
    iButtonWriter* writer = malloc(sizeof(iButtonWriter));
    writer->host = host;
    return writer;
}

void ibutton_writer_free(iButtonWriter* writer) {
    free(writer);
}

iButtonWriterResult ibutton_writer_write(iButtonWriter* writer, iButtonKey* key) {
    iButtonWriterResult result = iButtonWriterNoDetect;

    osKernelLock();
    bool blank_present = onewire_host_reset(writer->host);
    osKernelUnlock();

    if(blank_present) {
        switch(ibutton_key_get_type(key)) {
        case iButtonKeyDS1990:
            result = writer_write_DS1990(writer, key);
        default:
            break;
        }
    }

    return result;
}

void ibutton_writer_start(iButtonWriter* writer) {
    furi_hal_power_enable_otg();
    onewire_host_start(writer->host);
}

void ibutton_writer_stop(iButtonWriter* writer) {
    onewire_host_stop(writer->host);
    furi_hal_power_disable_otg();
}
